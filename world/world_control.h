#ifndef WORLD_CONTROL_H
#define WORLD_CONTROL_H

void request_thrust(unsigned int id, float amt);
void request_yaw(unsigned int id, float amt);
void request_pitch(unsigned int id, float amt);
void request_roll(unsigned int id, float amt);

#endif /* end of include guard: WORLD_CONTROL_H */
