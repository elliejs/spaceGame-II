#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <limits.h>

#include <libssh/server.h>

#include "../math/vector_3d.h"
#include "../utils/semaphore.h"
#include "../nblessings/nblessings.h"
#include "../world_db/world_db.h"
#include "render.h"
#include "render_daemon.h"
#include "../color/palette_helper.h"

typedef
struct pixel_job_s {
  SGVec rot_y_sin;
  SGVec rot_y_cos;

  SGVec rot_x_sin;
  SGVec rot_x_cos;

  unsigned int x;
  unsigned int y;

  struct pixel_job_s * next;
}
pixel_job_t;

static
struct {
  bool active;
  unsigned int id;

  pthread_t pid;
  pthread_t pixel_worker_pids[NUM_THREADS];
  int sem;
  pthread_mutex_t mtx;

  int width;
  int height;
  float quarter_rot_x;
  float quarter_rot_y;
  float rot_finder_x;
  float rot_finder_y;
  float half_fov_x;
  float half_fov_y;

  pixel_t * framebuffer;
  unsigned char * stream_buffer;

  unsigned int num_colors;
  oklab_t * colors;

  pixel_job_t * pixel_jobs;
  world_snapshot_t snapshot;

  ssh_channel channel;
}
render_client;

static
const struct timespec fps_ts = (struct timespec) {
  .tv_sec = 0L,
  .tv_nsec = 1000000000L / FPS
};

static inline
SGVec3D_t create_rays(orientation_t orientation, SGVec rots_sin_x, SGVec rots_cos_x, SGVec rots_sin_y, SGVec rots_cos_y) {
  return rot_vec3d(
    rots_sin_y,
    rots_cos_y,
    orientation.right,
    rot_vec3d(
      rots_sin_x,
      rots_cos_x,
      orientation.up,
      orientation.forward
    )
  );
}

static
void * pixel_task(void * nothing) {
  for (;;) {
    // printf("a\n");
    SEM_WAIT(render_client.sem, 0);
    // printf("b\n");
    pthread_testcancel();
    // printf("c\n");

    pixel_job_t * job = NULL;
    // printf("d\n");
    MTX_LOCK(render_client.mtx);
    // printf("e\n");
    job = render_client.pixel_jobs;
    render_client.pixel_jobs = render_client.pixel_jobs->next;
    MTX_UNLOCK(render_client.mtx);
    // printf("f\n");

    if (!job) continue;
    // printf("g\n");

    SGVec3D_t rays = create_rays(render_client.snapshot.self->ship.orientation, job->rot_x_sin, job->rot_x_cos, job->rot_y_sin, job->rot_y_cos);

    // printf("pixel [y: %.2u, x: %.2u]: fore: %u, back: %u, shape: %u\n", job->y, job->x, pixel.fore, pixel.back, pixel.shape);

    raw_pixel_t raw_pixel = rays_to_pixel(rays, &(render_client.snapshot));
    // printf("h\n");

    pixel_t pixel = (pixel_t) {
      .fore = closest_color_index(raw_pixel.fore, render_client.colors, render_client.num_colors),
      .back = closest_color_index(raw_pixel.back, render_client.colors, render_client.num_colors),
      .shape = raw_pixel.shape
    };
    // printf("i\n");


    render_client.framebuffer[job->y * render_client.width + job->x] = pixel;
    // printf("j\n");

    free(job);
    // printf("k\n");

    SEM_POST(render_client.sem, 1);
    // printf("l\n");

  }
  printf("dead\n");
}

static
void enqueue_render() {
  if (render_client.pixel_jobs != NULL) {
    printf("Alert: Unfinished framebuffer? Jobs remaining\n");
  }

  int post = 0;

  for (int y = 0; y < render_client.height; y++) {
    const float rot_y = y * render_client.rot_finder_y - render_client.half_fov_y;

    const float rot_y_neg = rot_y - render_client.quarter_rot_y;
    const float rot_y_neg_sin = sinf(rot_y_neg);
    const float rot_y_neg_cos = cosf(rot_y_neg);

    const float rot_y_pos = rot_y + render_client.quarter_rot_y;
    const float rot_y_pos_sin = sinf(rot_y_pos);
    const float rot_y_pos_cos = cosf(rot_y_pos);

    const SGVec rot_y_sin = SGVec_Load_Array(((float[]) {rot_y_pos_sin, rot_y_pos_sin, rot_y_neg_sin, rot_y_neg_sin}));
    const SGVec rot_y_cos = SGVec_Load_Array(((float[]) {rot_y_pos_cos, rot_y_pos_cos, rot_y_neg_cos, rot_y_neg_cos}));

    for (int x = 0; x < render_client.width; x++) {
      const float rot_x = x * render_client.rot_finder_x - render_client.half_fov_x;

      const float rot_x_neg = rot_x - render_client.quarter_rot_x;
      const float rot_x_neg_sin = sinf(rot_x_neg);
      const float rot_x_neg_cos = cosf(rot_x_neg);

      const float rot_x_pos = rot_x + render_client.quarter_rot_x;
      const float rot_x_pos_sin = sinf(rot_x_pos);
      const float rot_x_pos_cos = cosf(rot_x_pos);

      const SGVec rot_x_sin = SGVec_Load_Array(((float[]) {rot_x_neg_sin, rot_x_pos_sin, rot_x_neg_sin, rot_x_pos_sin}));
      const SGVec rot_x_cos = SGVec_Load_Array(((float[]) {rot_x_neg_cos, rot_x_pos_cos, rot_x_neg_cos, rot_x_pos_cos}));

      pixel_job_t * job = malloc(sizeof(pixel_job_t));
      *job = (pixel_job_t) {
        .rot_y_sin = rot_y_sin,
        .rot_y_cos = rot_y_cos,

        .rot_x_sin = rot_x_sin,
        .rot_x_cos = rot_x_cos,

        .x = x,
        .y = y
      };

      MTX_LOCK(render_client.mtx);
      job->next = render_client.pixel_jobs;
      render_client.pixel_jobs = job;
      MTX_UNLOCK(render_client.mtx);

      SEM_POST(render_client.sem, 0);
      post++;
    }
  }

  // printf("enqueued %d pixels\n", post);
}

static inline
void blit() {
  for (int i = render_client.width * render_client.height; i > 0; ) {
    short x = i > SHRT_MAX - 1 ? SHRT_MAX - 1 : i;
    // printf("wait4 %i\n", x);
    SEM_WAITVAL(render_client.sem, 1, x);
    i -= x;
  }
  pthread_testcancel();
  // printf("SPLAT!\n");
  unsigned int len = rasterize_frame(render_client.framebuffer, render_client.width * render_client.height, render_client.width, render_client.stream_buffer);
  ssh_channel_write(render_client.channel, render_client.stream_buffer, len);
}

static
void * render_task(void * nothing) {
  for (;;) {
    render_client.snapshot = request_snapshot(render_client.id);
    enqueue_render();
    // printf("queued\n");
    nanosleep(&fps_ts, NULL);
    // printf("shlept\n");
    blit();
    // printf("blitted\n");
    for(int i = 0; i < CUBE_NUM; i++) {
      free(render_client.snapshot.chunks[i]->objects);
      free(render_client.snapshot.chunks[i]->lights);
      free(render_client.snapshot.chunks[i]);
    }
  }
}

#define DEG2RAD(X) (((float) M_PI / (float) 180.) * (float) (X))

static
const float FOV_RAD = DEG2RAD(FOV);

void aspect_ratio_properties(int width, int height) {
  render_client.width = width;
  render_client.height = height;
  const int largest_dim = width > (2 * height) ? width : (2 * height);
  const float x_ratio = (float) width / (float) largest_dim;
  const float y_ratio = ((float) 2. * (float) height) / largest_dim;

  render_client.rot_finder_x = (x_ratio * FOV_RAD) / (float) width;
  render_client.rot_finder_y = (y_ratio * FOV_RAD) / (float) height;
  render_client.quarter_rot_x = render_client.rot_finder_x / (float) 4.;
  render_client.quarter_rot_y = render_client.rot_finder_y / (float) 4.;
  render_client.half_fov_x = x_ratio * (FOV_RAD / (float) 2.);
  render_client.half_fov_y = y_ratio * (FOV_RAD / (float) 2.);

  render_client.framebuffer = malloc(width * height * sizeof(pixel_t));
  render_client.stream_buffer = malloc(MAX_FRAMEBUFFER_SIZE(width, height) * sizeof(char));
}

void render_daemon(int width, int height, unsigned int max_colors, ssh_channel channel, unsigned int id) {
  render_client.id = id;
  aspect_ratio_properties(width, height);

  render_client.channel = channel;

  render_client.num_colors = 0;
  render_client.colors = malloc(max_colors * sizeof(oklab_t));

  //INSTALL COLORS HERE
  install_color((oklab_t) {
    .l = 0.,
    .a = 0.,
    .b = 0.
  }, max_colors, render_client.colors, &(render_client.num_colors));
  for (int j = -7; j < 8; j++) {
    for (int k = -7; k < 8; k++) {
      install_color((oklab_t) {
        .l = 5.,
        .a = j * (1. / (float) 7.),
        .b = k * (1. / (float) 7.)
      }, max_colors, render_client.colors, &(render_client.num_colors));
    }
  }

  unsigned int header_len;
  unsigned char * header_data = nblessings_header_data(render_client.colors, render_client.num_colors, &header_len);

  // printf("max_colors: %u, num_colors: %u\n", max_colors, render_client.num_colors);
  // printf("header_len: %u, header: %s\n", header_len, header_data);

  ssh_channel_write(render_client.channel, header_data, header_len);

  render_client.pixel_jobs = NULL;

  MTX_INIT(render_client.mtx);
  SEM_INIT(render_client.sem, 2);
  SEM_SETVAL(render_client.sem, 0, 0);
  SEM_SETVAL(render_client.sem, 1, 0);
  for (int i = 0; i < NUM_THREADS; i++) pthread_create(render_client.pixel_worker_pids + i, NULL, pixel_task, (void *) NULL);
  pthread_create(&(render_client.pid), NULL, render_task, (void *) NULL);

  render_client.active = true;
}

void end_render_daemon() {
  if (!render_client.active) return;

  printf("cancelling all jobs\n");
  for (int i = 0; i < NUM_THREADS; i++) pthread_cancel(render_client.pixel_worker_pids[i]);
  SEM_POSTVAL(render_client.sem, 0, NUM_THREADS);
  // MTX_UNLOCK(render_client.mtx);
  for (int i = 0; i < NUM_THREADS; i++) pthread_join(render_client.pixel_worker_pids[i], NULL);

  printf("cancelling main render loop\n");
  pthread_cancel(render_client.pid);
  SEM_POSTVAL(render_client.sem, 1, render_client.width * render_client.height);
  // MTX_UNLOCK(render_client.mtx);
  pthread_join(render_client.pid, NULL);
  // while (render_client.pixel_jobs) {}

  printf("destroying sem\n");
  SEM_DESTROY(render_client.sem);
  printf("destroying mtx\n");
  MTX_DESTROY(render_client.mtx);

  free(render_client.colors);
  free(render_client.framebuffer);
  free(render_client.stream_buffer);

  //TODO something is wrong here
  //DONE
  while (render_client.pixel_jobs != NULL) {
    // printf("%p\n", render_client.pixel_jobs);
    pixel_job_t * job = render_client.pixel_jobs;
    render_client.pixel_jobs = job->next;
    free(job);
  }

  // TODO
  // unsigned int footer_len;
  // unsigned char * footer_data = nblessings_footer_data(, &header_len);
  // ssh_channel_write(render_client.channel, footer_data, footer_len);

  render_client.active = false;
}
