#ifndef SSH_DAEMON_H
#define SSH_DAEMON_H

#include "ssh_client.h"

#define MAX_CLIENTS 256
#define RETRY_AFTER 5
#define MAX_AUTH_ATTEMPTS 3
#define BINDADDR "127.0.0.1"
#define BINDPORT "2222"
#define KEYS_FOLDER "ssh/keys/"

typedef struct ssh_server_s {
  ssh_client_t slots[MAX_CLIENTS];
  atomic_bool active[MAX_CLIENTS];
}
ssh_server_t;

void ssh_daemon(char const * bindaddr);

#endif /* end of include guard: SSH_DAEMON_H */
