# High-load C Chat Server

Implemented TCP chat server on C with sqlite3 for storage User data. This project easy for scaling and can handling around ten thousands simultaneous connections.

---

## Project Structure
````
├── include // Header files.
│   ├── config.h // Some constants.
│   ├── network.h
│   ├── registration.h
│   ├── requests.h
│   └── user.h
├── src // Source files.
│   ├── network.c // Low-level socket management.
│   ├── registration.c // Auth logic.
│   ├── requests.c // User requests processing.
│   ├── server.c // Main entry point and event loop.
│   └── user.c // Operations with database.
├── .gitignore
├── Dockerfile
├── LICENSE.txt
├── Makefile
└── README.md
````
---

## Usage Guide

1. Compile and Start:
   ```bash
   make
   ./bin/server
2. Connect via telnet:
   ```bash
   telnet localhost 3490
3. Commands:
- Registration: Send:
   ```bash
   :0:username:password:
- Login: Send:
   ```bash
   :1:username:password:
- Messaging: After Login into account every message will send to each connected user.

- Command separator is '\n', this can change in config.h as CMD_SEP.
---

## Roadmap
- Add private messaging between two clients.
- Store messages in database.
- Hashing messages and passwords in database.
- Create a Dockerfile for easy deployment in any environment. ✅
- Switch from 'poll()' to 'epoll()' or it equivalent.
- Implement WebSocket handshake and framing to support browser clients.

---




