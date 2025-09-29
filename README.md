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

## Conceptos que se aprenden “en el chiste”

- Multiplexación de clientes en **un solo hilo** usando `poll()`
- Manejo de **sockets non-blocking** sin perder mensajes
- Construcción de **buffers FIFO** por cliente (`sendBuf` y `recvBuf`)
- Implementación de un **mini parser IRC en tiempo real**
- Gestión de **canales, broadcast y mensajes privados** sin un cliente propio

---
