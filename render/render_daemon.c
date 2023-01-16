#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <limits.h>

#include <libssh/server.h>

#include "../math/vector_3d.h"
#include "../utils/semaphore.h"
#include "../nblessings/nblessings.h"
#include "../world/world_server.h"
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

  bool telomere;

  struct pixel_job_s * prev;
}
pixel_job_t;

static
struct {
  bool active;
  unsigned int id;

  pthread_t pid;
  pthread_t pixel_worker_pids[NUM_THREADS];
  int job_sem;
  pthread_mutex_t job_mtx;
  pthread_cond_t job_cond;
  pixel_job_t * pixel_jobs_front;
  pixel_job_t * pixel_jobs_back;

  int width;
  int height;
  pthread_mutex_t dim_mtx;
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

  world_snapshot_t snapshot;

  ssh_channel channel;
}
render_client = {
  .framebuffer = NULL,
  .stream_buffer = NULL,
  .pixel_jobs_back = NULL,
  .active = false
};

static
const struct timespec fps_ts = (struct timespec) {
  .tv_sec = 0L,
  .tv_nsec = 1000000000L / FPS
};

static inline
SGVec3D_t create_rays(orientation_t orientation, SGVec rots_sin_x, SGVec rots_cos_x, SGVec rots_sin_y, SGVec rots_cos_y) {
  SGVec4D_t rot_quat_y = prepare_rot_quat(rots_sin_y, rots_cos_y, orientation.right);
  SGVec4D_t rot_quat_x = prepare_rot_quat(rots_sin_x, rots_cos_x, orientation.up);
  return rot_vec3d(
    rot_quat_y,
    rot_vec3d(
      rot_quat_x,
      orientation.forward
    ));
}

static
void * pixel_task(void * nothing) {
  int tid = (int)nothing;
  // printf("%d disable cancel\n", tid);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  // printf("%d disable cancel success\n", tid);
  for (;;) {
    // printf("%d wait on sem 0\n", tid);
    SEM_WAIT(render_client.job_sem, 0);
    // printf("%d wait success\n", tid);
    // printf("%d enable cancel\n", tid);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // printf("%d enable cancel success\n", tid);
    pthread_testcancel();
    // printf("%d test cancel\n", tid);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    pixel_job_t * job = NULL;
    for (;;) {
      // printf("%d lock mtx?\n", tid);
      MTX_LOCK(&(render_client.job_mtx));
      // printf("%d lock mtx success\n", tid);
      while ((job = render_client.pixel_jobs_back) == NULL) {
        printf("%d start waiting\n", tid);
        COND_WAIT(&(render_client.job_cond), &(render_client.job_mtx));
        printf("%d end waiting\n", tid);
      }
      render_client.pixel_jobs_back = job->prev;
      // COND_SIGNAL(&(render_client.job_cond));
      MTX_UNLOCK(&(render_client.job_mtx));
      // printf("%d mtx unlock\n", tid);

      if (job->telomere) {
        // printf("%d Telomere\n", tid);
        break;
      }

      SGVec3D_t rays = create_rays(render_client.snapshot.self->ship.orientation, job->rot_x_sin, job->rot_x_cos, job->rot_y_sin, job->rot_y_cos);

      // printf("pixel [y: %.2u, x: %.2u]: fore: %u, back: %u, shape: %u\n", job->y, job->x, pixel.fore, pixel.back, pixel.shape);
      raw_pixel_t raw_pixel = rays_to_pixel(rays, &(render_client.snapshot));
      pixel_t pixel = (pixel_t) {
        .fore = closest_color_index(raw_pixel.fore, render_client.colors, render_client.num_colors),
        .back = closest_color_index(raw_pixel.back, render_client.colors, render_client.num_colors),
        .shape = raw_pixel.shape
      };
      render_client.framebuffer[job->y * render_client.width + job->x] = pixel;
      free(job);
    }
    // printf("%d telomere escaped\n", tid);
    free(job);
    SEM_POST(render_client.job_sem, 1);
    printf("%d posted its telomere\n", tid);
    // MTX_LOCK(&(render_client.job_mtx));
    // printf("%d telomere locked\n", tid);
    // COND_SIGNAL(&(render_client.job_cond));
    // printf("%d telomere signalled\n", tid);
    // MTX_UNLOCK(&(render_client.job_mtx));
    // printf("%d telomere unlocked\n", tid);
  }
  printf("[render_client %u]: Thread's dead Jim!\n", render_client.id);
}

static
void enqueue_render() {
  MTX_LOCK(&(render_client.job_mtx));
  if (render_client.pixel_jobs_back != NULL) {
    printf("Alert: Unfinished framebuffer? Jobs remaining\n");
  }
  MTX_UNLOCK(&(render_client.job_mtx));

  SEM_POSTVAL(render_client.job_sem, 0, NUM_THREADS);
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
        .y = y,

        .telomere = false,
        .prev = NULL
      };

      MTX_LOCK(&(render_client.job_mtx));
      if (render_client.pixel_jobs_back)
        render_client.pixel_jobs_front->prev = job;
      else
        render_client.pixel_jobs_back = job;

      render_client.pixel_jobs_front = job;
      // printf("new job posted\n");
      COND_SIGNAL(&(render_client.job_cond));
      MTX_UNLOCK(&(render_client.job_mtx));
    }

    // MTX_LOCK(&(render_client.job_mtx));
    // printf("Broadcasting\n");
    // COND_BROADCAST(&(render_client.job_cond));
    // MTX_UNLOCK(&(render_client.job_mtx));
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pixel_job_t * job = malloc(sizeof(pixel_job_t));
    *job = (pixel_job_t) {
      .telomere = true,
      .prev = NULL
    };
    MTX_LOCK(&(render_client.job_mtx));
    if (render_client.pixel_jobs_back)
      render_client.pixel_jobs_front->prev = job;
    else
      render_client.pixel_jobs_back = job;

    render_client.pixel_jobs_front = job;
    printf("telomere job queued\n");
    // COND_SIGNAL(&(render_client.job_cond));
    MTX_UNLOCK(&(render_client.job_mtx));
  }
  MTX_LOCK(&(render_client.job_mtx));
  printf("Broadcasting\n");
  COND_BROADCAST(&(render_client.job_cond));
  MTX_UNLOCK(&(render_client.job_mtx));
}


static
const float FOV_RAD = DEG2RAD(FOV);

void render_daemon_request_dimensions(int width, int height) {
  MTX_LOCK(&(render_client.dim_mtx));
  printf("nuts\n");
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

  render_client.framebuffer = realloc(render_client.framebuffer, width * height * sizeof(pixel_t));
  render_client.stream_buffer = realloc(render_client.stream_buffer, MAX_FRAMEBUFFER_SIZE(width, height) * sizeof(char));
  MTX_UNLOCK(&(render_client.dim_mtx));
}

static inline
void blit() {
  printf("A\n");
  SEM_WAITVAL(render_client.job_sem, 1, NUM_THREADS);
  printf("B\n");
  destroy_snapshot(&(render_client.snapshot));
  // for (int i = 0; i < render_client.width * render_client.height; i++) {
  //   if (render_client.framebuffer[i].fore >= render_client.num_colors || render_client.framebuffer[i].back >= render_client.num_colors) {
  //   }
  // }
  unsigned int len = rasterize_frame(render_client.framebuffer, render_client.width * render_client.height, render_client.width, render_client.stream_buffer);
  unsigned int written = 0;
  printf("would like to write %u\n", len);
  while (written < len) {
    printf("written (before: %u)\n", written);
    written += ssh_channel_write(render_client.channel, render_client.stream_buffer + written, len - written);
    printf("written (after: %u)\n", written);
  }
  printf("d\n");
}

static
void * render_task(void * nothing) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  unsigned int id = render_client.id;
  for (;;) {
    printf("[render_client %u]: top loop\n", id);
    render_client.snapshot = request_snapshot(id);
    printf("[render_client %u]: snapshotted\n", id);
    MTX_LOCK(&(render_client.dim_mtx));
    printf("[render_client %u]: dims locked\n", id);
    enqueue_render();
    printf("[render_client %u]: queued\n", id);
    nanosleep(&fps_ts, NULL);
    printf("[render_client %u]: shlept\n", id);
    blit();
    MTX_UNLOCK(&(render_client.dim_mtx));
    printf("[render_client %u]: blitted\n", id);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  }
}

void render_daemon(int width, int height, unsigned int max_colors, ssh_channel channel, unsigned int id) {
  printf("starting render daemon\n");
  render_client.id = id;

  MTX_INIT(&(render_client.dim_mtx));

  render_daemon_request_dimensions(width, height);

  render_client.channel = channel;

  render_client.num_colors = 0;
  render_client.colors = malloc(max_colors * sizeof(oklab_t));

  // INSTALL COLORS HERE
  install_palette(id, duel_init(), max_colors, render_client.colors, &(render_client.num_colors));

  unsigned int header_len;
  unsigned char * header_data = nblessings_header_data(render_client.colors, render_client.num_colors, &header_len);

  // printf("max_colors: %u, num_colors: %u\n", max_colors, render_client.num_colors);
  // printf("header_len: %u, header: %s\n", header_len, header_data);

  ssh_channel_write(render_client.channel, header_data, header_len);

  MTX_INIT(&(render_client.job_mtx));
  COND_INIT(&(render_client.job_cond));
  render_client.job_sem = SEM_INIT(2);
  SEM_SETVAL(render_client.job_sem, 0, 0);
  SEM_SETVAL(render_client.job_sem, 1, 0);
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_create(render_client.pixel_worker_pids + i, NULL, pixel_task, (void*)i);

  pthread_create(&(render_client.pid), NULL, render_task, NULL);

  render_client.active = true;
}

void end_render_daemon() {
  if (!render_client.active) return;

  printf("cancelling main render loop\n");
  pthread_cancel(render_client.pid);
  pthread_join(render_client.pid, NULL);

  printf("cancelling all jobs\n");
  for (int i = 0; i < NUM_THREADS; i++) pthread_cancel(render_client.pixel_worker_pids[i]);
  SEM_POSTVAL(render_client.job_sem, 0, NUM_THREADS);
  for (int i = 0; i < NUM_THREADS; i++) pthread_join(render_client.pixel_worker_pids[i], NULL);

  printf("destroying job_sem\n");
  SEM_DESTROY(render_client.job_sem);
  printf("destroying mtx dim\n");
  MTX_DESTROY(&(render_client.dim_mtx));
  printf("destroying mtx main\n");
  MTX_DESTROY(&(render_client.job_mtx));
  printf("destroying cond main\n");
  COND_DESTROY(&(render_client.job_cond));

  free(render_client.colors);
  free(render_client.framebuffer);
  free(render_client.stream_buffer);

  // TODO
  // unsigned int footer_len;
  // unsigned char * footer_data = nblessings_footer_data(, &header_len);
  // ssh_channel_write(render_client.channel, footer_data, footer_len);

  render_client.active = false;
}
