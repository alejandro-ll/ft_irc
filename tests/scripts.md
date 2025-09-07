# Comandos rápidos
# Terminal 1
./ircserv 6667 mipass

# Terminal 2
nc -C 127.0.0.1 6667
PASS mipass
NICK ana
USER ana 0 * :Ana
JOIN #bar

# Terminal 3
    nc -C 127.0.0.1 6667
    PASS mipass
    NICK bob
    USER bob 0 * :Bob
    JOIN #bar
    PRIVMSG #bar :hola!
