#ifndef ASTROGAMMON_CLIENT_H
#define ASTROGAMMON_CLIENT_H

#include <stdint.h>

bool join_game(char * game_tag);
int handle_input(void *data, uint32_t len, int is_stderr, void *userdata);

#endif /* end of include guard: ASTROGAMMON_CLIENT_H */
