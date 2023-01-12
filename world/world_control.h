#ifndef WORLD_CONTROL_H
#define WORLD_CONTROL_H

typedef
enum vision_mode_e {
  STANDARD = 0,
  RADAR = 1,
}
vision_mode_t;

void request_thrust(unsigned int id, float amt);
void request_yaw(unsigned int id, float amt);
void request_pitch(unsigned int id, float amt);
void request_roll(unsigned int id, float amt);

void request_vision(unsigned int id, vision_mode_t mode);

#endif /* end of include guard: WORLD_CONTROL_H */
