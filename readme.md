# Codename: SpaceGame II


https://user-images.githubusercontent.com/46575669/176509053-182580f1-dd4f-48f7-85e5-03351e57a98f.mov



### Running your own SpaceGame Server

---

Building:


---

Dependencies:
- [libssh](https://www.libssh.org/)

---


run the server as: `./spaceGame [optional IPv4 bindaddr]`

run the client as: `ssh jelly@[127.0.0.1 or bindaddr supplied to server] -p 2222`, password is 'toast'

---

Requested Features:
- dvorak mapping option
- **[DONE]** ~~script for server owners to generate required keys~~
  - **[FIX]** [keygen.sh](keygen.sh)
- **[DONE]** ~~runtime bindaddr selection~~
- **[DONE]** ~~Makefile for ease of building and configuration.~~
  - **[FIX]** [Makefile](Makefile)
