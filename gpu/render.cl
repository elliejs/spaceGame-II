// https://www.khronos.org/files/opencl30-reference-guide.pdf
// https://www.eriksmistad.no/getting-started-with-opencl-and-gpu-computing/

global const double fov = radians(45.);
global const double3 SGFrame_RIGHT 	 = (double3) {1., 0., 0.};
global const double3 SGFrame_UP 	 = (double3) {0., 1., 0.};
global const double3 SGFrame_FORWARD = (double3) {0., 0., 1.};

double4 multq(double4 a, double4 b) {
	return (double4) {
		.w = -1.*dot(a, b),
		.xyz = cross(a, b),
	};
}

double3 rot3(double4 rotq, double3 vec) {
	const double4 vecq = (double4)(0., vec);
	const double4 qp = multq(rotq, vecq);
	rotq.xyz *= -1;
	return multq(qp, rotq).xyz;
}

double3 rot_view(double4 attitude, double2 rot) {
	const double4 y_rot.xyz = SGFrame_RIGHT * sincos(rot.y, &(y_rot.w));
	const double4 x_rot.xyz = SGFrame_UP * sincos(rot.x, &(x_rot.w));
	const double4 total_rot = multq(attitude, multq(x_rot, y_rot));
	return rot3(total_rot, SGFrame_FORWARD);
}


typedef
struct ray_march_result_s {
	object_t * object;
}
ray_march_result_t;

ray_march_result_t ray_march(double3 pos, double3 dir, world_snapshot_t * world_state) {
	double tot_dist = 0.;

	object_t * hit_object = NULL;
	for (unsigned int steps = 0; steps < MAX_RAYMARCH_STEPS; i++) {
		double dist_step = DOUBLE_MAX;
		for (unsigned int c = 0; c < NUM_CHUNKS; c++) {
			chunk_t * chunk = world_state->chunks + c;
			for (unsigned int o = 0; o < chunk->num_objects; o++) {
				object_t * object = chunk->objects + o;

				double dist_candidate = object->distance(object, c, world_state->time);
				if (dist_candidate < dist_step) {
					dist_step = dist_candidate;
					hit_obj = object;
				}
			}
		}

		if (fabs(dist_step) <= HIT_DIST)
			return (ray_march_result_t) {
				.object = hit_obj;
			};
		else
			tot_dist += dist_step;

		if (tot_dist > MAX_DIST)
			return (ray_march_result_t) {
				.object = NULL;
			}
	}
}


kernel
void render(global const world_snapshot_t * world_state, global ushort3 * raw_framebuffer) {
	const size_t id_0 = get_global_id(0);
	const size_t id_1 = get_global_id(1);
	const size_t size_0 = get_global_size(0);
	const size_t size_1 = get_global_size(1);

	const double2 dim = (double2)(id_0, id_1);
	const double max_dim = fmax(size_0, size_1 * 2);
	const double2 rot = dim / max_dim * fov;

	ship_t * ship = (ship_t *) world_state->self;

	const double3 dir = rot_view(ship.attitude, rot);
	framebuffer[id_0 * size_0 + id_1] = ray_march(ship.origin, dir, world_state);
}
