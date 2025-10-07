#!/bin/bash
# diagnostic_test.sh - Diagnóstico del problema

SERVER_HOST="127.0.0.1"
SERVER_PORT="6667"
PASSWORD="mypass"

echo "🔍 DIAGNÓSTICO DEL SERVIDOR IRC"
echo "================================"

# Verificar servidor
echo "1. Verificando servidor..."
if ps aux | grep -v grep | grep -q -E "(valgrind.*ircserv|./ircserv)"; then
    echo "   ✅ Servidor ejecutándose"
else
    echo "   ❌ Servidor NO ejecutándose"
    exit 1
fi

# Verificar puerto
echo ""
echo "2. Verificando puerto $SERVER_PORT..."
if netstat -tulpn 2>/dev/null | grep -q ":$SERVER_PORT"; then
    echo "   ✅ Puerto $SERVER_PORT en uso"
else
    echo "   ❌ Puerto $SERVER_PORT NO en uso"
    exit 1
fi

# Prueba de conexión básica
echo ""
echo "3. Prueba de conexión TCP básica..."
if nc -z $SERVER_HOST $SERVER_PORT 2>/dev/null; then
    echo "   ✅ Conexión TCP exitosa"
else
    echo "   ❌ No se puede conectar al puerto"
    exit 1
fi

# Prueba de eco simple
echo ""
echo "4. Prueba de eco (¿responde el servidor?)..."
echo "   Enviando: TEST"
response=$(echo "TEST" | timeout 2 nc $SERVER_HOST $SERVER_PORT | head -1)
if [ -n "$response" ]; then
    echo "   ✅ Servidor RESPONDE: $response"
else
    echo "   ❌ Servidor NO RESPONDE - conexión silenciosa"
fi

# Prueba de comando IRC básico
echo ""
echo "5. Prueba comando IRC básico..."
echo "   Enviando: NICK testuser"
response=$(echo "NICK testuser" | timeout 2 nc $SERVER_HOST $SERVER_PORT | head -1)
if [ -n "$response" ]; then
    echo "   ✅ Respuesta a NICK: $response"
else
    echo "   ❌ Sin respuesta a comando NICK"
fi

# Prueba con PASS
echo ""
echo "6. Prueba autenticación completa..."
{
    echo "PASS $PASSWORD"
    sleep 0.5
    echo "NICK diagnostic"
    sleep 0.5  
    echo "USER diag 0 * :Diagnostic User"
    sleep 1
} | timeout 5 nc -v $SERVER_HOST $SERVER_PORT 2>&1 | head -10

# Verificar clientes conectados
echo ""
echo "7. Clientes actualmente conectados..."
netstat -tpn 2>/dev/null | grep ":$SERVER_PORT" | grep "ESTABLISHED" | wc -l | xargs echo "   Conexiones establecidas:"

echo ""
echo "📋 RESUMEN DEL DIAGNÓSTICO:"
echo "============================"
echo "Si ves 'Servidor NO RESPONDE' arriba, el problema es que:"
echo "1. El servidor acepta conexiones pero NO envía respuestas"
echo "2. Revisa la implementación de los comandos IRC en tu servidor"
echo "3. Verifica que esté enviando mensajes de bienvenida y respuestas"
echo ""
echo "Comandos IRC que DEBERÍA responder:"
echo "   - Al conectar: NOTICE * :*** Looking up your hostname..."
echo "   - Al NICK: 001 nick :Welcome message"
echo "   - Al USER: Mensaje de registro completo"
