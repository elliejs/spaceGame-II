#ifndef RENDER_DAEMON_H
#define RENDER_DAEMON_H

#define NUM_THREADS 64
#define FPS 10

void render_daemon(int width, int height);
void end_render_daemon();

#endif
