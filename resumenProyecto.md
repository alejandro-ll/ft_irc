El proyecto `alejandro-ll/ft_irc` es una implementación de un servidor IRC (Internet Relay Chat) en C++.

---

## 0. Un servidor IRC (Internet Relay Chat)

- Es un programa que permite a múltiples usuarios conectarse y comunicarse en tiempo real a través de **canales o salas de chat**. Funciona bajo el **protocolo IRC**, que define cómo se envían mensajes, comandos y notificaciones entre clientes y servidores.

### Componentes básicos de un servidor IRC

0.1. **Clientes IRC**

- Programas que los usuarios utilizan para conectarse, por ejemplo:

  - `irssi` (CLI)
  - `HexChat` (GUI)
  - `nc` o `telnet` (para pruebas)

- Los clientes envían mensajes y comandos al servidor.

  0.2. **Servidor IRC**

- Escucha conexiones TCP en un puerto (normalmente 6667 o 6697 para SSL)
- Gestiona múltiples usuarios simultáneamente
- Administra canales, permisos, y mensajes privados o públicos
- Implementa comandos estándar como:

  - `JOIN <canal>` → unirse a un canal
  - `PRIVMSG <usuario/canal>` → enviar mensaje
  - `NICK <nombre>` → cambiar nombre de usuario

  0.3. **Canales**

- Son como “salas de chat” donde los usuarios pueden interactuar
- Los servidores mantienen listas de usuarios y permisos por canal

  0.4. **Protocolo IRC**

- Define los **mensajes de control y comunicación** entre cliente y servidor
- Todo es texto plano, con líneas terminadas en `\r\n`
- Ejemplo de mensaje de un cliente:

  ```
  NICK frromero
  USER frromero 0 * :Frromero User
  JOIN #inception
  PRIVMSG #inception :Hola a todos!
  ```

---

## 1. Estructura de ficheros

**Raíz del repositorio:**

- `Makefile`: Script para compilar el proyecto.
- `include/`: Contiene los archivos de cabecera (.hpp).
- `src/`: Contiene el código fuente principal.

---

### Contenido de `/include` (cabeceras)

- `Channel.hpp`: Define la clase y lógica relacionada con los canales de IRC.
- `Client.hpp`: Define la clase para representar a un cliente conectado.
- `Parser.hpp`: Encargado del análisis de los comandos recibidos del cliente.
- `Server.hpp`: Define la clase principal que representa el servidor IRC.

---

### Contenido de `/src` (código fuente)

- `main.cpp`: Punto de entrada de la aplicación. Inicializa el servidor.
- `/core`: (Subdirectorio) Suele contener la lógica central del programa (el listado específico no se mostró, pero normalmente aquí estarían las implementaciones de Server, Client, Channel, etc.).
- `/handlers`: (Subdirectorio) Normalmente contiene funciones para manejar comandos IRC concretos (JOIN, PRIVMSG, NICK, etc.).

---

### Contenido de `/tests`

- `scripts.md`: Contiene scripts o instrucciones para testear el servidor.

---

## 2. Funcionamiento genérico

- El usuario compila el proyecto usando el `Makefile`.
- Al ejecutar el binario, se inicia el servidor IRC (con la configuración que se indique).
- Los clientes pueden conectarse al servidor, autenticarse, unirse a canales, enviar mensajes, etc., siguiendo el protocolo IRC.

---

## 3. Funcionamiento profundo y cronológico

### 1. Compilación y Preparación

- El `Makefile` compila los archivos fuente de `src/`, utilizando las cabeceras de `include/`, para generar el ejecutable del servidor.

### 2. Inicio del Servidor

- Se ejecuta `main.cpp`, que instancia la clase `Server` (definida en `Server.hpp`/implementada en `src/core`).
- El servidor abre un socket, escucha conexiones y gestiona los eventos de la red.

### 3. Conexión de Clientes

- Al conectarse un cliente, se crea una instancia de `Client` (definida en `Client.hpp`).
- El servidor gestiona el registro del cliente, autenticación y almacenamiento de su información.

### 4. Interpretación de Comandos

- Cada mensaje del cliente es analizado por la clase `Parser` (`Parser.hpp`).
- El parser identifica el comando (NICK, JOIN, PRIVMSG, PART, etc.) y sus argumentos.

### 5. Gestión de Canales y Mensajes

- Para comandos relacionados con canales, se usan las clases y lógica de `Channel.hpp`.
- Los canales gestionan listas de usuarios, modos, topic, etc.
- El servidor distribuye los mensajes a los clientes correspondientes según el canal.

### 6. Manejo de Eventos y Comandos

- Los archivos en `src/handlers/` implementan la respuesta a cada comando IRC.
- El servidor envía respuestas y notificaciones a los clientes según el protocolo.

### 7. Pruebas

- En `tests/scripts.md` se documentan scripts y escenarios de prueba para validar el comportamiento del servidor.
