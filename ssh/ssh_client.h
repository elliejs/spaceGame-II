#ifndef SSH_CLIENT_H
#define SSH_CLIENT_H

#include <stdatomic.h>
#include <stdbool.h>

#include <libssh/callbacks.h>

typedef
enum game_mode_e {
  ASTROGAMMON,
  SPACEGAME
}
game_mode_t;

typedef
struct ssh_client_s {
  //static id info
  unsigned int id;
  off_t user_index;
  pid_t pid;
  atomic_bool * active;
  bool pleaseKill;

  //ssh params
  bool pre_channel;
  bool authenticated;
  unsigned int auth_attempts;

  //static state info
  ssh_session session;
  struct ssh_server_callbacks_struct ssh_client_session_callbacks;
  ssh_channel channel;
  struct ssh_channel_callbacks_struct ssh_client_channel_callbacks;
  ssh_event event;

  game_mode_t mode;
}
ssh_client_t;

#endif /* end of include guard: SSH_CLIENT_H */
