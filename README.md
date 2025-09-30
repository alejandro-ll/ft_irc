# Proyecto ft_irc

Este proyecto implementa un **servidor IRC simple en C++** para comprender los conceptos básicos de:

- Sockets TCP
- Multiplexación de clientes
- Parsing de comandos IRC

**Nota:** No incluye un cliente propio. Se espera que los clientes sean externos, como `nc`, `irssi` o cualquier otro cliente IRC.

---

## Conceptos clave que se aprenden

### 1️⃣ Servidor TCP y sockets

- El servidor crea un **socket de escucha (`listen_fd`)** en un puerto dado (ej.: 6667).
- Funciones principales:

  - `bind()` → ata el socket a todas las interfaces (`0.0.0.0`)
  - `listen()` → pone el socket en modo escucha
  - `setNonBlocking()` → evita que bloquee el hilo principal

---

### 2️⃣ Multiplexación de clientes con `poll()`

- Cada cliente se representa con un **FD** y un objeto `Client`.
- Se mantiene un vector `pfds` de `pollfd`:

  - `events` → eventos que queremos observar (lectura, escritura)
  - `revents` → eventos ocurridos

- `poll()` permite **gestionar múltiples clientes simultáneamente** sin necesidad de hilos extra.

---

### 3️⃣ Recepción de mensajes (`POLLIN`)

- Cuando un cliente envía datos, el FD marca `POLLIN`.
- El servidor llama a `handleRead()`:

  - Acumula bytes en `Client.recvBuf`
  - Detecta líneas terminadas en `\r\n`
  - Llama a `onLine()` → `handleCommand()`

- Cada línea se procesa **en tiempo real**, interpretando comandos IRC como `PRIVMSG`, `JOIN`, etc.

---

### 4️⃣ Envío de mensajes (`POLLOUT`)

- Los sockets son **non-blocking**, así que no siempre se puede enviar todo de golpe.
- Cada cliente tiene un **sendBuf (buffer FIFO)** con mensajes pendientes:

  - `sendTo()` agrega datos a `sendBuf` y marca `POLLOUT` para ese FD
  - Cuando `poll()` detecta `POLLOUT`, se llama a `handleWrite()`:

    - Envía bytes desde el inicio del buffer
    - Si no se puede enviar todo, se mantiene el resto para la siguiente iteración

- Esto asegura que:

  - Ningún mensaje se pierda
  - El servidor no se bloquee

---

### 5️⃣ Broadcast y canales

- Cada canal tiene un conjunto de miembros: `Channel.members`
- `broadcastToChannel()` envía mensajes a todos los miembros **excepto al emisor**
- El servidor gestiona automáticamente **a quién enviar los mensajes** según los FDs de cada cliente

---

### 6️⃣ Cliente “invisible”

- El servidor **nunca actúa como cliente**.
- Los usuarios se conectan mediante clientes externos, ej.:

```bash
nc 127.0.0.1 6667
```

- Operaciones:

  - `read()` → recibe mensajes del servidor
  - `write()` → envía mensajes al servidor

- Todo ocurre en línea dentro del servidor, sin necesidad de código cliente propio.

---

## Flujo completo resumido

```
Cliente real (nc, irssi)
        │ escribe mensaje
        ▼
Servidor (FD del cliente)
        │ handleRead() → parsea línea → onLine() → handleCommand()
        │
        ├─ sendTo(c, msg) → sendBuf → handleWrite() → FD del cliente
        └─ broadcastToChannel(...) → sendBuf de otros clientes → handleWrite()
```

**Notas importantes:**

- Cada FD se gestiona individualmente
- `POLLIN` → este cliente tiene datos para leer
- `POLLOUT` → este cliente puede recibir datos sin bloquear
- `sendBuf` → mensajes pendientes de enviar en orden FIFO

---

## Cómo ejecutar

```bash
./ft_irc <puerto> <contraseña>
```

**Ejemplo:**

```bash
./ft_irc 6667 mypass
```

- El servidor se queda en **bucle infinito** (`srv.run()`) escuchando clientes
- Los clientes externos pueden conectarse con:

```bash
nc 127.0.0.1 6667
```

- Todo mensaje enviado por el servidor aparecerá automáticamente en la terminal del cliente, gracias al loop interno de `nc`.

---

## Requisitos obligatorios

- No usar `fork()`, ni múltiples hilos.
- Solo un `poll()` (o equivalente) para todas las operaciones.
- Comunicación vía TCP/IP (IPv4 o IPv6).
- Cliente de referencia debe conectarse sin errores.
- Implementar comandos IRC mínimos:
  - `PASS`, `NICK`, `USER`, `JOIN`, `PRIVMSG`
  - Comandos de operador: `KICK`, `INVITE`, `TOPIC`, `MODE`

---
## Conceptos que se aprenden

- Multiplexación de clientes en **un solo hilo** usando `poll()`
- Manejo de **sockets non-blocking** sin perder mensajes
- Construcción de **buffers FIFO** por cliente (`sendBuf` y `recvBuf`)
- Implementación de un **mini parser IRC en tiempo real**
- Gestión de **canales, broadcast y mensajes privados** sin un cliente propio

---


# handler

## 🧠 ¿Qué es un handler de comando?

Un *handler* es una función que se encarga de **procesar un comando específico** enviado por un cliente. En este caso, cada comando IRC (como `NICK`, `JOIN`, `PRIVMSG`, etc.) tiene su propio método en la clase `Server`.


## 🧩 Lista de comandos y su propósito

### 🔐 `cmdPASS`
- **Propósito**: Verifica la contraseña del cliente.
- **Uso típico**: `PASS <contraseña>`
- **Lógica esperada**:
  - Comparar con `Server::password`.
  - Marcar al cliente como autenticado si es correcta.

---

### 🧑 `cmdNICK`
- **Propósito**: Establece o cambia el apodo del cliente.
- **Uso típico**: `NICK <nuevo_nick>`
- **Lógica esperada**:
  - Verificar que el nick no esté en uso.
  - Asignarlo al cliente.
  - Notificar a otros usuarios si ya está en un canal.

---

### 👤 `cmdUSER`
- **Propósito**: Proporciona información del usuario (nombre real, etc.).
- **Uso típico**: `USER <username> <hostname> <servername> <realname>`
- **Lógica esperada**:
  - Guardar los datos en el objeto `Client`.
  - Intentar registrar al cliente si ya envió `PASS` y `NICK`.

---

### 🔁 `cmdPING`
- **Propósito**: Verifica que el cliente siga conectado.
- **Uso típico**: `PING <token>`
- **Lógica esperada**:
  - Responder con `PONG <token>`.

---

### ❌ `cmdQUIT`
- **Propósito**: El cliente se desconecta voluntariamente.
- **Uso típico**: `QUIT :<mensaje>`
- **Lógica esperada**:
  - Notificar a otros usuarios.
  - Cerrar la conexión y limpiar recursos.

---

### 📥 `cmdJOIN`
- **Propósito**: El cliente entra a un canal.
- **Uso típico**: `JOIN #canal`
- **Lógica esperada**:
  - Crear el canal si no existe.
  - Añadir al cliente a la lista de miembros.
  - Enviar la lista de usuarios y el topic del canal.

---

### 📤 `cmdPART`
- **Propósito**: El cliente sale de un canal.
- **Uso típico**: `PART #canal`
- **Lógica esperada**:
  - Eliminar al cliente del canal.
  - Notificar a los demás miembros.

---

### 💬 `cmdPRIVMSG`
- **Propósito**: Enviar un mensaje privado a un usuario o canal.
- **Uso típico**: `PRIVMSG <destino> :<mensaje>`
- **Lógica esperada**:
  - Si el destino es un canal, reenviar a todos los miembros.
  - Si es un usuario, reenviar directamente.

---

### ⚙️ `cmdMODE`
- **Propósito**: Cambiar modos de usuario o canal (como operador, privado, etc.).
- **Uso típico**: `MODE #canal +o <nick>`
- **Lógica esperada**:
  - Validar permisos.
  - Aplicar el cambio y notificar.

---

### 📝 `cmdTOPIC`
- **Propósito**: Ver o cambiar el tema del canal.
- **Uso típico**: `TOPIC #canal :<nuevo tema>`
- **Lógica esperada**:
  - Si no hay argumento, mostrar el tema actual.
  - Si hay argumento, cambiarlo (si tiene permisos).

---

### 📩 `cmdINVITE`
- **Propósito**: Invitar a un usuario a un canal.
- **Uso típico**: `INVITE <nick> #canal`
- **Lógica esperada**:
  - Verificar que el canal existe y que el usuario tiene permisos.
  - Enviar invitación al usuario.

---

### 🦵 `cmdKICK`
- **Propósito**: Expulsar a un usuario de un canal.
- **Uso típico**: `KICK #canal <nick> :<razón>`
- **Lógica esperada**:
  - Verificar que el emisor es operador.
  - Eliminar al usuario del canal.
  - Notificar a todos los miembros.

---
