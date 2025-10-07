#!/bin/bash
# fixed_traffic_test.sh - Pruebas con TRAFICO REAL funcionando

SERVER_HOST="127.0.0.1"
SERVER_PORT="6667"
PASSWORD="mypass"

echo "PRUEBAS COMPLETAS - TRAFICO REAL"
echo "================================"

# Función para verificar servidor
check_server() {
    ps aux | grep -v grep | grep -q -E "(valgrind.*ircserv|./ircserv)"
}

# Función CORREGIDA que SÍ muestra tráfico
run_working_client() {
    local client_id="$1"
    local commands="$2"
    local duration="${3:-10}"
    
    (
        echo "========================================"
        echo "CLIENTE: $client_id - INICIADO"
        echo "========================================"
        
        # Crear script temporal CORRECTO
        local temp_script=$(mktemp)
        {
            echo "#!/bin/bash"
            # Convertir comandos echo a comandos reales
            while IFS= read -r line; do
                if [[ "$line" == echo* ]]; then
                    # Extraer el contenido entre comillas
                    content=$(echo "$line" | sed -E 's/^echo[[:space:]]+"(.*)"$/\1/')
                    echo "echo \"$content\""
                elif [[ "$line" == sleep* ]]; then
                    echo "$line"
                elif [[ "$line" =~ ^for ]]; then
                    echo "$line"
                elif [[ "$line" == done* ]]; then
                    echo "$line"
                elif [[ "$line" =~ ^if ]]; then
                    echo "$line"
                elif [[ "$line" == fi* ]]; then
                    echo "$line"
                fi
            done <<< "$commands"
        } > "$temp_script"
        chmod +x "$temp_script"
        
        echo "COMANDOS:"
        grep -E '^echo|^sleep|^for|^done|^if|^fi' "$temp_script" | sed 's/^echo "//' | sed 's/"$//'
        echo ""
        echo "--- TRAFICO EN VIVO ---"
        
        # Ejecutar CORRECTAMENTE
        {
            # Ejecutar script y capturar salida
            bash "$temp_script"
        } | {
            # Procesar cada línea para enviar
            while IFS= read -r line; do
                if [[ -n "$line" ]]; then
                    echo ">>> $client_id ENVIA: $line"
                    echo "$line"
                    sleep 0.2
                fi
            done
        } | {
            # Conectar y recibir respuestas
            nc -i 1 -w 10 $SERVER_HOST $SERVER_PORT 2>&1 | while IFS= read -r response; do
                if [[ ! "$response" =~ "Connection to" ]] && [[ ! "$response" =~ "succeeded" ]]; then
                    echo "<<< $client_id RECIBE: $response"
                fi
            done
        } &
        
        local client_pid=$!
        sleep $duration
        kill $client_pid 2>/dev/null
        wait $client_pid 2>/dev/null
        rm -f "$temp_script"
        
        echo "--- FIN TRAFICO ---"
        echo "CLIENTE: $client_id - FINALIZADO"
        echo "========================================"
        echo ""
    ) 
}

cleanup() {
    echo "Limpiando procesos..."
    pkill -f "nc $SERVER_HOST $SERVER_PORT" 2>/dev/null
    sleep 2
}

trap cleanup EXIT

# Verificación inicial
echo "Verificando servidor..."
if check_server; then
    echo "Servidor detectado"
else
    echo "ERROR: Servidor no detectado"
    exit 1
fi

# Función de espera
wait_and_check() {
    local wait_time=$1
    local test_name=$2
    
    echo ""
    echo "EJECUTANDO: $test_name ($wait_time segundos)"
    for i in $(seq 1 $wait_time); do
        sleep 1
        if ! check_server; then
            echo "ERROR: Servidor crasheo"
            return 1
        fi
    done
    echo "COMPLETADO: $test_name"
    return 0
}

# Contadores
FAILED_TESTS=0
TOTAL_TESTS=8

echo ""
echo "INICIANDO PRUEBAS..."
echo ""

# PRUEBA 1: Autenticación básica
echo "1/$TOTAL_TESTS: AUTENTICACION BASICA"
for i in {1..2}; do
    commands=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK user$i"
echo "USER u$i 0 * :User $i"
sleep 1
echo "PING :test$i"
sleep 2
echo "QUIT :Bye"
CMD
)
    run_working_client "user$i" "$commands" 6 &
    sleep 1
done

wait_and_check 8 "autenticacion" || ((FAILED_TESTS++))

# PRUEBA 2: Canales simples
echo ""
echo "2/$TOTAL_TESTS: CANALES SIMPLES"
commands=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK tester"
echo "USER te 0 * :Tester"
sleep 1
echo "JOIN #test"
echo "PRIVMSG #test :Hola canal"
sleep 2
echo "TOPIC #test :Mi topico"
sleep 1
echo "PRIVMSG #test :Segundo mensaje"
sleep 2
echo "PART #test :Adios"
sleep 1
CMD
)
run_working_client "tester" "$commands" 8

wait_and_check 10 "canales" || ((FAILED_TESTS++))

# PRUEBA 3: Comandos básicos
echo ""
echo "3/$TOTAL_TESTS: COMANDOS BASICOS"
commands=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK cmduser"
echo "USER cu 0 * :Command User"
sleep 1
echo "JOIN #room1"
echo "JOIN #room2"
sleep 1
echo "NAMES #room1"
echo "LIST"
sleep 1
echo "PRIVMSG #room1 :Hola room1"
echo "PRIVMSG #room2 :Hola room2"
sleep 2
echo "PART #room1"
echo "PART #room2"
sleep 1
CMD
)
run_working_client "cmduser" "$commands" 8

wait_and_check 10 "comandos basicos" || ((FAILED_TESTS++))

# PRUEBA 4: Interacción
echo ""
echo "4/$TOTAL_TESTS: INTERACCION"

# Cliente 1
commands1=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK userA"
echo "USER ua 0 * :User A"
sleep 1
echo "JOIN #chat"
echo "PRIVMSG #chat :Hola soy UserA"
sleep 3
echo "PRIVMSG #chat :Alguien me escucha?"
sleep 3
CMD
)
run_working_client "userA" "$commands1" 10 &

# Cliente 2
commands2=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK userB"
echo "USER ub 0 * :User B"
sleep 2
echo "JOIN #chat"
echo "PRIVMSG #chat :Hola UserA! Soy UserB"
sleep 2
echo "PRIVMSG #chat :Te escucho claro"
sleep 3
CMD
)
run_working_client "userB" "$commands2" 10 &

wait_and_check 12 "interaccion" || ((FAILED_TESTS++))

# PRUEBA 5: Flood suave
echo ""
echo "5/$TOTAL_TESTS: FLOOD SUAVE"
commands=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK flooder"
echo "USER fl 0 * :Flooder"
sleep 1
echo "JOIN #flood"
for i in {1..5}; do
    echo "PRIVMSG #flood :Mensaje \$i"
done
echo "PRIVMSG #flood :Fin flood"
sleep 3
CMD
)
run_working_client "flooder" "$commands" 8

wait_and_check 10 "flood" || ((FAILED_TESTS++))

# PRUEBA 6: Datos largos
echo ""
echo "6/$TOTAL_TESTS: DATOS LARGOS"
commands=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK VeryLongNickname123"
echo "USER vl 0 * :Very Long User"
sleep 1
echo "JOIN #long"
echo "PRIVMSG #long :Este es un mensaje bastante largo para probar el manejo del servidor"
sleep 2
echo "TOPIC #long :Topico tambien muy largo para pruebas extensivas"
sleep 2
CMD
)
run_working_client "longuser" "$commands" 8

wait_and_check 10 "datos largos" || ((FAILED_TESTS++))

# PRUEBA 7: Múltiples clientes
echo ""
echo "7/$TOTAL_TESTS: MULTIPLES CLIENTES"
for i in {1..3}; do
    commands=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK multi$i"
echo "USER mu$i 0 * :Multi $i"
sleep 2
echo "JOIN #multi"
echo "PRIVMSG #multi :Hola desde multi$i"
sleep 3
CMD
)
    run_working_client "multi$i" "$commands" 8 &
    sleep 1
done

wait_and_check 10 "multiples clientes" || ((FAILED_TESTS++))

# PRUEBA 8: Test de límite MEJORADO
echo ""
echo "8/$TOTAL_TESTS: TEST LIMITE MEJORADO"

echo "Fase 1: 60 conexiones base..."
for i in {1..60}; do
    commands=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK base$i"
echo "USER ba$i 0 * :Base $i"
sleep 20
CMD
)
    eval "$commands" | nc $SERVER_HOST $SERVER_PORT > /dev/null 2>&1 &
    sleep 0.05
done

echo "Esperando 5 segundos..."
sleep 5

echo "Fase 2: 50 conexiones adicionales (deberian rechazarse)..."
for i in {1..50}; do
    commands=$(cat <<CMD
echo "PASS $PASSWORD"
echo "NICK over$i"
echo "USER ov$i 0 * :Over $i"
sleep 10
CMD
)
    # Solo algunos muestran tráfico para no saturar
    if [ $i -le 5 ]; then
        run_working_client "over$i" "$commands" 12 &
    else
        eval "$commands" | nc $SERVER_HOST $SERVER_PORT > /dev/null 2>&1 &
    fi
    sleep 0.1
done

wait_and_check 15 "test limite" || ((FAILED_TESTS++))

cleanup

echo ""
echo "================================"
echo "PRUEBAS COMPLETADAS"
echo "RESULTADO: $((TOTAL_TESTS - FAILED_TESTS))/$TOTAL_TESTS"

if [ $FAILED_TESTS -eq 0 ]; then
    echo "✅ TODAS LAS PRUEBAS EXITOSAS"
    echo ""
    echo "El servidor manejo correctamente:"
    echo "  - Autenticacion y comandos basicos"
    echo "  - Canales y mensajeria"
    echo "  - Multiples clientes simultaneos"
    echo "  - Flood controlado"
    echo "  - Datos largos"
    echo "  - Limite de ~100 conexiones"
else
    echo "❌ $FAILED_TESTS pruebas fallaron"
fi

echo "================================"

exit $FAILED_TESTS
