Este proyecto implementa un servidor IRC simple en C++ para entender cómo funcionan los conceptos básicos de sockets, multiplexación de clientes y parsing de comandos.  No incluye un cliente propio: se espera que los clientes sean externos, como nc, irssi o cualquier cliente IRC.
Conceptos clave que se aprenden
1️⃣ Servidor TCP y sockets
- El servidor crea un socket de escucha (listen_fd) en un puerto dado (ej: 6667). - bind() → lo ata a todas las interfaces (0.0.0.0). - listen() → lo pone en modo escucha. - setNonBlocking() → para que no bloquee el hilo principal.
2️⃣ Multiplexación de clientes con poll()
- Cada cliente se representa con un FD y un objeto Client. - Se mantiene un vector pfds de pollfd:   - events → qué eventos queremos observar (lectura, escritura)   - revents → qué eventos ocurrieron - poll() permite gestionar muchos clientes simultáneamente sin hilos extra.
3️⃣ Recepción de mensajes (POLLIN)
- Cuando un cliente envía datos, el FD marca POLLIN. - El servidor llama a handleRead():   - Acumula bytes en Client.recvBuf   - Detecta líneas terminadas en \r\n   - Llama a onLine() → handleCommand() - Cada línea se procesa en tiempo real, interpretando comandos IRC como PRIVMSG, JOIN, etc.
4️⃣ Envío de mensajes (POLLOUT)
- Los sockets son non-blocking, así que no siempre se puede enviar todo de golpe. - Cada cliente tiene un sendBuf (buffer FIFO) con los mensajes pendientes. - sendTo() agrega datos a sendBuf y marca POLLOUT para ese FD. - Cuando poll() detecta POLLOUT, se llama a handleWrite():   - Envía bytes desde el inicio del buffer   - Si no se puede enviar todo, se mantiene el resto para la siguiente iteración - Esto asegura que ningún mensaje se pierda y que el servidor no se bloquee.
5️⃣ Broadcast y canales
- Cada canal tiene un conjunto de miembros (Channel.members). - broadcastToChannel() envía un mensaje a todos los miembros, excepto el emisor. - El servidor gestiona automáticamente a quiénes enviar mensajes basándose en los FDs de cada cliente.
6️⃣ Cliente “invisible” en este proyecto
- El servidor nunca actúa como cliente. - El CMD del usuario se conecta con algo externo (nc 127.0.0.1 6667) y:   - read() → recibe los mensajes del servidor   - write() → envía los mensajes al servidor - Por eso, todo ocurre “en línea” dentro del servidor, sin necesidad de código cliente.
Flujo completo resumido
Cliente real (nc, irssi)         │ escribe mensaje         ▼ Servidor (fd del cliente)         │ handleRead() → parsea línea → onLine() → handleCommand()         │         ├─ sendTo(c, msg) → sendBuf → handleWrite() → FD del cliente         └─ broadcastToChannel(...) → sendBuf de otros clientes → handleWrite()  - Cada FD se gestiona individualmente. - POLLIN → este cliente tiene datos para leer - POLLOUT → este cliente puede recibir datos sin bloquear - sendBuf → mensajes pendientes de enviar en orden FIFO
Cómo ejecutar
```bash ./ft_irc <puerto> <contraseña> ```  Ejemplo: ```bash ./ft_irc 6667 mypass ```  - El servidor se queda en bucle infinito (srv.run()) escuchando clientes. - Los clientes externos pueden conectarse con: ```bash nc 127.0.0.1 6667 ``` - Todo mensaje que el servidor envíe aparecerá automáticamente en la terminal del cliente, gracias al loop interno de nc.
Conceptos que se aprenden “en el chiste”
- Cómo multiplexar clientes en un solo hilo usando poll(). - Cómo manejar sockets non-blocking sin perder mensajes. - Cómo construir buffers FIFO para cada cliente (sendBuf y recvBuf). - Cómo implementar un mini parser IRC en tiempo real. - Cómo gestionar canales, broadcast y mensajes privados sin un cliente propio.
