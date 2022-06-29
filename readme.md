# Codename: SpaceGame II

https://user-images.githubusercontent.com/46575669/176509053-182580f1-dd4f-48f7-85e5-03351e57a98f.mov

## About

*Codename: SpaceGame II* is a labour of love I have been developing over the past few years. What started out as a dream to pay homage to the telnet based [MUDs](https://en.wikipedia.org/wiki/MUD) of the early 1980s internet, has steadily grown into a fantastic world of its own. Through several iterations and countless hours, *Codename: SpaceGame II* has grown from an in-terminal ray-marcher using [ncurses](https://invisible-island.net/ncurses/ncurses.html), to an online multiplayer game accessible through SSH.

Along the way I've learned how to write multi-processed and multi-threaded server code, [SPOT](https://en.wikipedia.org/wiki/Single_source_of_truth) server architecture netcode so every client stays in sync with each other, the world around them, and the server, and architecture specific SIMD optimization intrinsics for the physics and rendering backend. The whole project, as it is the vehicle on which I've accelerated my interest in graphics and networking after enjoying a few graduate courses in college, is written from the ground up utilizing as few libraries as possible. *Codename: SpaceGame II* is what happens when you build a universe from scratch.

Over the course of the project's development I've rewritten many libraries, implemented others, and optimized yet more. The original ncurses implementation was rewritten from scratch into a streamlined library I call [nblessings](nblessings) which allows for faster frame-rate blitting. I've written a highly optimized [Quaternion](https://en.wikipedia.org/wiki/Quaternion) [implementation of rotation](math/vector_3d.h) which only takes 28 SIMD operations to complete, an intense [caching algorithm](world) built from self-balancing [aa-trees](https://en.wikipedia.org/wiki/AA_tree) and unordered lists. As it stands, [libssh](https://www.libssh.org/) is the ONLY third-party library in the whole project.


## Running your own SpaceGame Server

### Building:

There are many build targets available with make.

- **make**: With no args, make creates a medium performance build without cleaning.
- **all**: Cleans the directory and rebuilds medium performance.
- **optimized**: Builds at optimization level 3, and with fast-math.
- **debug**: Builds with debug flags enabled, which add some more server-side print statements.
- **clean**: removes all object files.

---

### Dependencies:

- [libssh](https://www.libssh.org/)

---

### Running

run the server as: `./spaceGame [optional IPv4 bindaddr]`

run the client as: `ssh jelly@[127.0.0.1 or bindaddr supplied to server] -p 2222`, password is 'toast'

---

### Supported Platforms

- ARMv8 NEON

---

Requested Features:
- dvorak mapping option
- **[DONE]** ~~script for server owners to generate required keys~~
  - **[FIX]** [keygen.sh](keygen.sh)
- **[DONE]** ~~runtime bindaddr selection~~
- **[DONE]** ~~Makefile for ease of building and configuration.~~
  - **[FIX]** [Makefile](Makefile)
- **[TODO]** Move from SIMD to Vulkan, to make more platforms available
