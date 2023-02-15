#ifndef RENDER_DAEMON_H
#define RENDER_DAEMON_H

#include <libssh/server.h>

#define NUM_THREADS 4
#define FPS 10

void render_daemon(int width, int height, unsigned int max_colors, ssh_channel channel, unsigned int id);
void end_render_daemon();
void render_daemon_request_dimensions(int width, int height);
#endif /* end of include guard: RENDER_DAEMON_H */
