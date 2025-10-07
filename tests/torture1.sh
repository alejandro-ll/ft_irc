#!/bin/bash
# torture_test_fixed.sh

SERVER_HOST="127.0.0.1"
SERVER_PORT="6667"
PASSWORD="mypass"

echo "🧪 INICIANDO PRUEBAS TORTURA IRC - CONEXIONES MASIVAS"

# Prueba 1: Conexiones con autenticación básica
echo "🔌 Prueba 1: 50 conexiones autenticadas..."
for i in {1..50}; do
    (
        echo "PASS $PASSWORD"
        echo "NICK user$i"
        echo "USER u$i 0 * :Test User $i"
        # Mantener conexión activa
        sleep 5
    ) | nc $SERVER_HOST $SERVER_PORT &
    echo "Cliente $i conectado (PID: $!)"
done

echo "⏳ Esperando 7 segundos para observar estabilidad..."
sleep 7

# Verificar procesos activos
echo "📊 Procesos nc activos:"
pgrep -c nc

# Limpiar
echo "🧹 Limpiando procesos..."
pkill -f "nc 127.0.0.1 6667"

echo "✅ Prueba de conexiones masivas completada"
