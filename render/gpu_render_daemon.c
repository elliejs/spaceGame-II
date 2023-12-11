#include <stdio.h>
#include <stdlib.h>
#include "../utils/gpu.h"
#include "../utils/semaphore.h"
#include "../world/world_server.h"
#include "../color/palette_helper.h"
#include "../nblessings/nblessings.h"
#include <libssh/libssh.h>

#define FPS 10


static
struct {
  bool active;
  unsigned int id;

  pthread_t pid;

  unsigned int width;
  unsigned int height;
  pthread_mutex_t dim_mtx;

  cl_double3 * raw_framebuffer;
  pixel_t * encoded_framebuffer;
  unsigned char * stream_buffer;

  unsigned int num_colors;
  oklab_t * colors;

  world_snapshot_t snapshot;

  ssh_channel channel;
  gpu_context_t context;
}
render_client = {
  .framebuffer = NULL,
  .stream_buffer = NULL,
  .active = false
};

static
const struct timespec fps_ts = (struct timespec) {
  .tv_sec = 0L,
  .tv_nsec = 1000000000L / FPS
};

static inline
void blit() {
  unsigned int len = rasterize_frame(render_client.encoded_framebuffer, render_client.width * render_client.height, render_client.width, render_client.stream_buffer);
  unsigned int written = 0;
  printf("would like to write %u\n", len);
  while (written < len) {
    printf("written (before: %u)\n", written);
    written += ssh_channel_write(render_client.channel, render_client.stream_buffer + written, len - written);
    printf("written (after: %u)\n", written);
  }
}


static
void * render_task(void * nothing) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  unsigned int id = render_client.id;
  for (;;) {
printf("[render_client %u]: top loop\n", id);
    request_snapshot(&(render_client.snapshot), id);
    gpu_set_args(&render_client.context, 0, sizeof(world_snapshot_t), &render_client.snapshot);
    destroy_snapshot(&(render_client.snapshot));
printf("[render_client %u]: snapshotted\n", id);
    MTX_LOCK(&(render_client.dim_mtx));
printf("[render_client %u]: dims locked\n", id);
    gpu_run_kernel(&render_client.context, 2, (const size_t []) {render_client.width, render_client.height}, NULL);
    gpu_get_args(&render_client.context, 1,
                 render_client.width * render_client.height * 4 * sizeof(cl_double3), &render_client.raw_framebuffer);
    gpu_flush(&render_client.context);
printf("[render_client %u]: queued\n", id);
    nanosleep(&fps_ts, NULL); //TODO: make this a better time barrier
printf("[render_client %u]: slept\n", id);
    blit();
    MTX_UNLOCK(&(render_client.dim_mtx));
printf("[render_client %u]: blitted\n", id);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  }
}


void render_daemon_request_dimensions(int width, int height) {
  MTX_LOCK(&(render_client.dim_mtx));
  printf("request_dims\n");
  render_client.width = width;
  render_client.height = height;

  render_client.raw_framebuffer = realloc(render_client.raw_framebuffer, width * height * 4 * sizeof(cl_double3));
  render_client.encoded_framebuffer = realloc(render_client.encoded_framebuffer, width * height * sizeof(pixel_t));
  render_client.stream_buffer = realloc(render_client.stream_buffer, MAX_FRAMEBUFFER_SIZE(width, height) * sizeof(char));
  MTX_UNLOCK(&(render_client.dim_mtx));
}

void render_daemon(int width, int height, unsigned int max_colors, ssh_channel channel, unsigned int id) {
  printf("starting render daemon\n");
  render_client.id = id;
  render_client.channel = channel;
  render_client.context = gpu_load_kernel("../gpu/render.cl", "render");
  gpu_allocate_args_RO(&render_client.context, sizeof(world_snapshot_t));
  MTX_INIT(&(render_client.dim_mtx));
  render_daemon_request_dimensions(width, height);


  // INSTALL COLORS HERE
  render_client.num_colors = 0;
  render_client.colors = malloc(max_colors * sizeof(oklab_t));
  install_palette(id, duel_init(), max_colors, render_client.colors, &(render_client.num_colors));

  unsigned int header_len;
  unsigned char * header_data = nblessings_header_data(render_client.colors, render_client.num_colors, &header_len);
  ssh_channel_write(render_client.channel, header_data, header_len);

  pthread_create(&(render_client.pid), NULL, render_task, NULL);

  render_client.active = true;
}

void end_render_daemon() {
  if (!render_client.active) return;

  printf("cancelling render kernel\n");
  gpu_destroy_context(&render_client.context);
  printf("cancelling main render loop\n");
  pthread_cancel(render_client.pid);
  pthread_join(render_client.pid, NULL);

  printf("destroying mtx dim\n");
  MTX_DESTROY(&(render_client.dim_mtx));

  free(render_client.colors);
  free(render_client.raw_framebuffer);
  free(render_client.encoded_framebuffer);
  free(render_client.stream_buffer);

  // TODO
  // unsigned int footer_len;
  // unsigned char * footer_data = nblessings_footer_data(, &header_len);
  // ssh_channel_write(render_client.channel, footer_data, footer_len);

  render_client.active = false;
}
