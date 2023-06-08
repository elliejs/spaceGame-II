#ifndef ASTROGAMMON_CLIENT_H
#define ASTROGAMMON_CLIENT_H

#include <stdint.h>
#include <libssh/callbacks.h>

bool join_game(char * game_tag);
int handle_input(void *data, uint32_t len, int is_stderr, void *userdata);
void start_astrogammon_client(int width, int height, int max_colors, ssh_channel channel, unsigned int id);
#endif /* end of include guard: ASTROGAMMON_CLIENT_H */
