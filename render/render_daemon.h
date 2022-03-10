#ifndef RENDER_DAEMON_H
#define RENDER_DAEMON_H

#define NUM_THREADS 64
#define FPS 10

void render_daemon(int width, int height, unsigned int max_colors, ssh_channel channel, unsigned int id);
void end_render_daemon();

#endif /* end of include guard: RENDER_DAEMON_H */
