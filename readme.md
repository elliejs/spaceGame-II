# Codename: SpaceGame II





<iframe width="560" height="315" src="https://www.youtube.com/embed/ghebc8kRrmA" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>


### Running your own SpaceGame Server
---

Compile:
```
gcc */*.c */*/*.c -lssh -lm -O3 -ffast-math -o spaceGame [-D SG_DEBUG]
```

Include the `-D SG_DEBUG` to get debug print statements and such.

---

Dependencies:
- [libssh](https://www.libssh.org/)

---

run the server as: `./spaceGame`

run the client as: `ssh jelly@127.0.0.1 -p 2222`, password is 'toast'

---

Requested Features:
- dvorak mapping option
- **[DONE]** ~~script for server owners to generate required keys~~
  - **[FIX]** [keygen.sh](keygen.sh)
