#!/bin/bash
# debug_valgrind_safe.sh - Pruebas con valgrind SIN terminar el servidor

SERVER_HOST="127.0.0.1"
SERVER_PORT="6667"
PASSWORD="mypass"

echo "🐛 PRUEBAS CON VALGRIND (SEGURO)"
echo "================================"

# Verificar que está compilado con -g
echo "🔍 Verificando símbolos de debug..."
if ! readelf --debug-dump=decodedline ./ircserv 2>/dev/null | grep -q "Server.cpp"; then
    echo "❌ Compila primero con: c++ -g -std=c++98 -Wall -Wextra -Werror *.cpp -o ircserv"
    exit 1
fi

echo ""
echo "💡 INSTRUCCIONES:"
echo "1. Ejecuta el servidor MANUALMENTE en otra terminal:"
echo "   valgrind --leak-check=full --track-origins=yes ./ircserv 6667 mypass"
echo ""
echo "2. Presiona ENTER cuando el servidor esté ejecutándose..."
read

echo "🧪 Iniciando pruebas automáticas..."
echo ""

# Limpiar sólo clientes de pruebas anteriores
pkill -f "nc $SERVER_HOST $SERVER_PORT" 2>/dev/null
sleep 1

# Prueba 1: Conexión simple
echo "🔹 Prueba 1: Conexión simple"
(
    echo "PASS $PASSWORD"
    echo "NICK testuser"
    echo "USER test 0 * :Test User"
    sleep 2
    echo "QUIT :Bye"
) | timeout 5 nc $SERVER_HOST $SERVER_PORT &
sleep 3

# Prueba 2: Comandos básicos
echo "🔹 Prueba 2: Comandos básicos"
(
    echo "PASS $PASSWORD"
    echo "NICK user2"
    echo "USER u2 0 * :User 2"
    sleep 1
    echo "JOIN #test"
    echo "PRIVMSG #test :Hola mundo"
    echo "PING :server"
    sleep 2
    echo "QUIT :Adiós"
) | timeout 6 nc $SERVER_HOST $SERVER_PORT &
sleep 4

# Prueba 3: Múltiples conexiones
echo "🔹 Prueba 3: 3 conexiones ligeras"
for i in {1..3}; do
    (
        echo "PASS $PASSWORD"
        echo "NICK client$i"
        echo "USER c$i 0 * :Client $i"
        sleep 3
        echo "QUIT :Done"
    ) | timeout 5 nc $SERVER_HOST $SERVER_PORT &
    sleep 0.5
done

echo "⏳ Esperando que terminen las pruebas..."
sleep 6

# Limpiar sólo los clientes de prueba
pkill -f "nc $SERVER_HOST $SERVER_PORT" 2>/dev/null

echo ""
echo "✅ PRUEBAS COMPLETADAS"
echo ""
echo "📋 MIRA LA TERMINAL DONDE EJECUTASTE VALGRIND PARA:"
echo "   - Memory leaks"
echo "   - Invalid reads/writes" 
echo "   - Errores de segmentación"
echo ""
echo "🛑 Para terminar el servidor manualmente: Ctrl+C en la otra terminal"
echo "========================================"
