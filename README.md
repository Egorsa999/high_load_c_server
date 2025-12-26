# High-load C Chat Server

Implemented TCP chat server on C with sqlite3 for storage User data. This project easy for scaling and can handling around ten thousands simultaneous connections.

---

## Project Structure
- `server.c`: Main entry point and event loop.
- `network.c/h`: Low-level socket management.
- `user.c/h`: Operations with database.
- `requests.c/h`: User requests processing.
- `registration.c/h`: Auth logic.
- `config.h`: some constants

---

## Usage Guide

1. Compile and Start:
   ```bash
   make
   ./server
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




