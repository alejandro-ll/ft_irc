#pragma once
#include <string>
#include <set>

struct Channel {
    std::string name, topic, key;
    bool inviteOnly;     // +i
    bool topicOpOnly;    // +t
    int  limit;          // +l; -1 = sin límite
    std::set<int> members, ops, invited;

    Channel() : inviteOnly(false), topicOpOnly(false), limit(-1) {}
    explicit Channel(const std::string& n)
        : name(n), inviteOnly(false), topicOpOnly(false), limit(-1) {}

    // === Getters / checks ===
    bool isMember(int fd)   const { return members.count(fd)   != 0; }
    bool isOp(int fd)       const { return ops.count(fd)       != 0; }
    bool isInvited(int fd)  const { return invited.count(fd)   != 0; }

    // === Setters para modos ===
    void setInviteOnly(bool on)      { inviteOnly = on; }
    void setTopicRestricted(bool on) { topicOpOnly = on; }
    void setKey(const std::string& k){ key = k; }
    void clearKey()                  { key.clear(); }
    void setLimit(int n)             { limit = n; }
    void clearLimit()                { limit = -1; }

    // === Gestión de miembros ===
    void addMember(int fd)   { members.insert(fd); invited.erase(fd); }
    void removeMember(int fd){ members.erase(fd); ops.erase(fd); invited.erase(fd); }
    void addOp(int fd)       { ops.insert(fd); }
    void invite(int fd)      { invited.insert(fd); }

    // === Validación de JOIN ===
    bool canJoin(int fd, const std::string* providedKey) const {
        if (limit >= 0 && (int)members.size() >= limit) return false;   // +l
        if (!key.empty()) {
            if (!providedKey || *providedKey != key) return false;      // +k
        }
        if (inviteOnly && invited.count(fd) == 0 && members.count(fd) == 0)
            return false;                                               // +i
        return true;
    }
};
