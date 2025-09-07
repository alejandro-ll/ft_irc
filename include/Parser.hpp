#pragma once
#include <vector>
#include <string>
#include <sstream>

struct Cmd { std::string verb; std::vector<std::string> args; };

inline Cmd parseIrcLine(const std::string& line) {
    Cmd out; std::string s = line, trailing;
    std::string::size_type p = s.find(" :");
    if (p != std::string::npos) { trailing = s.substr(p + 2); s.erase(p); }
    std::istringstream iss(s);
    iss >> out.verb;
    for (std::string tok; iss >> tok; ) out.args.push_back(tok);
    if (!trailing.empty()) out.args.push_back(trailing);
    for (size_t i=0;i<out.verb.size();++i) out.verb[i]=std::toupper(out.verb[i]);
    return out;
}
