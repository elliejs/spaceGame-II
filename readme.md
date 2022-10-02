# Codename: SpaceGame II

https://user-images.githubusercontent.com/46575669/176509053-182580f1-dd4f-48f7-85e5-03351e57a98f.mov

## About

*Codename: SpaceGame II* is a labour of love I have been developing over the past few years. What started out as a dream to pay homage to the telnet based [MUDs](https://en.wikipedia.org/wiki/MUD) of the early 1980s internet, has steadily grown into a fantastic world of its own. Through several iterations and countless hours, *Codename: SpaceGame II* has grown from an in-terminal ray-marcher using [ncurses](https://invisible-island.net/ncurses/ncurses.html), to an online multiplayer game accessible through SSH.

Along the way I've learned how to write multi-processed and multi-threaded server code, [SPOT](https://en.wikipedia.org/wiki/Single_source_of_truth) server architecture netcode so every client stays in sync with each other, the world around them, and the server, and architecture specific SIMD optimization intrinsics for the physics and rendering backend. The whole project, as the vehicle on which I've accelerated my interest in graphics and networking after enjoying a few graduate courses in college, is written from the ground up utilizing as few libraries as possible. *Codename: SpaceGame II* is what happens when you build a universe from scratch.

Over the course of the project's development I've rewritten many libraries, implemented others, and optimized yet more. The original ncurses implementation was rewritten from scratch into a streamlined library I call [nblessings](nblessings) which allows for faster frame-rate blitting. I've written a highly optimized [Quaternion](https://en.wikipedia.org/wiki/Quaternion) [implementation of rotation](math/vector_3d.h) which only takes 28 SIMD operations to complete, an intense chunk [caching algorithm](world) built from self-balancing [aa-trees](https://en.wikipedia.org/wiki/AA_tree) and unordered lists, and a client dispatcher with asynchronous event handling. As it stands, [libssh](https://www.libssh.org/) is the ONLY third-party library in the whole project.

The graphics pipeline is based around a ray marcher, which includes occlusive shadows, ambient and point lighting, and color blending and matching based off [oklab](https://bottosson.github.io/posts/oklab/); Björn Ottosson's numerically stable interpretation of the recent CAM16-UCS and CIELUV descriptions of color theory, combining speed and fidelity to the real world. The ray marcher is multithreaded, processing tasks from a pool of yet-to-be rendered pixels, which lets the CPU operate on all cores at the same time, drastically speeding up rendering times per frame.


## Running your own SpaceGame Server

### Building:

There are many build targets available with make.

- **make**: With no args, make creates a medium performance build without cleaning.
- **all**: Cleans the directory and rebuilds medium performance.
- **optimized**: Builds at optimization level 3, and with fast-math.
- **debug**: Builds with debug flags enabled, which add some more server-side print statements.
- **clean**: removes all object files.

### Dependencies:

- [libssh](https://www.libssh.org/)

### Running

- run the server as: `./spaceGame [optional IPv4 bindaddr]`

- run the client as: `ssh jelly@[127.0.0.1 or bindaddr supplied to server] -p 2222`, password is 'toast'

### Supported Platforms

- ARMv8 with NEON SIMD

---

## Feature Roadmap:

- [ ] **[MOVED]** ~~dvorak mapping option~~
- [X] ~~script for server owners to generate required keys~~
  - **[FIX]** [keygen.sh](keygen.sh)
- [X] ~~runtime bindaddr selection~~
- [X] ~~Makefile for ease of building and configuration~~
  - **[FIX]** [Makefile](Makefile)
- [ ] ~~Move from SIMD to Vulkan, to make more platforms available~~
  - **[WONT DO]** Since Vulkan requires so much boilerplate and GPU ray-marching isn't optimizable anyway, this has been forgone in favor of just buying more threads and cpus.
- [ ] Persistent user profiles.
  - [ ] user keymaps for Colemak, Dvorak, etc
  - [ ] save-states for location, health, inventory, etc
- [ ] Friend sharing and waypointing
