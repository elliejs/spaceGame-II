```
               ___           ___           ___           ___           ___     
              /\  \         /\  \         /\  \         /\  \         /\  \    
             /::\  \       /::\  \       /::\  \       /::\  \       /::\  \   
            /:/\ \  \     /:/\:\  \     /:/\:\  \     /:/\:\  \     /:/\:\  \  
           _\:\~\ \  \   /::\~\:\  \   /::\~\:\  \   /:/  \:\  \   /::\~\:\  \
          /\ \:\ \ \__\ /:/\:\ \:\__\ /:/\:\ \:\__\ /:/__/ \:\__\ /:/\:\ \:\__\
          \:\ \:\ \/__/ \/__\:\/:/  / \/__\:\/:/  / \:\  \  \/__/ \:\~\:\ \/__/
           \:\ \:\__\        \::/  /       \::/  /   \:\  \        \:\ \:\__\  
            \:\/:/  /         \/__/        /:/  /     \:\  \        \:\ \/__/  
             \::/  /                      /:/  /       \:\__\        \:\__\    
              \/__/                       \/__/         \/__/         \/__/    
                      ___           ___           ___           ___          
                     /\  \         /\  \         /\__\         /\  \         
                    /::\  \       /::\  \       /::|  |       /::\  \        
                   /:/\:\  \     /:/\:\  \     /:|:|  |      /:/\:\  \       
                  /:/  \:\  \   /::\~\:\  \   /:/|:|__|__   /::\~\:\  \      
                 /:/__/_\:\__\ /:/\:\ \:\__\ /:/ |::::\__\ /:/\:\ \:\__\     
                 \:\  /\ \/__/ \/__\:\/:/  / \/__/~~/:/  / \:\~\:\ \/__/     
                  \:\ \:\__\        \::/  /        /:/  /   \:\ \:\__\       
                   \:\/:/  /        /:/  /        /:/  /     \:\ \/__/       
                    \::/  /        /:/  /        /:/  /       \:\__\         
                     \/__/         \/__/         \/__/         \/__/         
                                    ___           ___   
                                   /\  \         /\  \  
                                   \:\  \        \:\  \
                                   /::\__\       /::\__\
                                __/:/\/__/    __/:/\/__/
                               /\/:/  /      /\/:/  /   
                               \::/__/       \::/__/    
                                \:\__\        \:\__\    
                                 \/__/         \/__/    
```
---
Compile:
```
gcc */*.c */*/*.c -lssh -lm -o spaceGame
```
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
