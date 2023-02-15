#ifndef GUI_H
#define GUI_H

#include <libssh/libssh.h>

#include "astrogammon_server.h"
#include "rulesets/ruleset.h"

void display_command_tray(game_phase_t phase);
bool start_astrogammon_gui(unsigned int width, unsigned int height, unsigned int max_colors, ssh_channel channel, unsigned int id);

#endif /* end of include guard: GUI_H */
