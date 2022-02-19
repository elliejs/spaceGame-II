#include <pthread.h>
#include <time.h>
#include <stdbool.h>

#include "render_daemon.h"

typedef
struct render_client_s {
  bool active;
  pthread_t pid;

  int width;
  int height;

  char * framebuffer;

  unsigned int num_colors;
  // luv_t * colors;
}
render_client_t;

static
render_client_t render_client;

static
const struct timespec fps_ts = (struct timespec) {
  .tv_sec = 0L,
  .tv_nsec = 1000000000L / FPS
};

static
void enqueue_render() {

}

static
void blit() {

}

static
void * render_task(void * data) {
  for(;;) {
    enqueue_render();
    nanosleep(&fps_ts, NULL);
    blit();
  }
}

void render_daemon(int width, int height) {
  render_client.width = width;
  render_client.height = height;
  pthread_create(&(render_client.pid), NULL, render_task, (void *) NULL);
  render_client.active = true;
}

void end_render_daemon() {
  if(!render_client.active) return;
  pthread_cancel(render_client.pid);
}
