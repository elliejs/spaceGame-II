#ifndef CALLBACKS_SSH_H
#define CALLBACKS_SSH_H

#include <libssh/callbacks.h>

int auth_callback(const char *prompt, char *buf, size_t len, int echo, int verify, void *userdata);
void log_callback(ssh_session session, int priority, const char *message, void *userdata);
void status_callback(void *userdata, float status);
void global_request_callback(ssh_session session, ssh_message message, void *userdata);
ssh_channel channel_open_request_x11_callback(ssh_session session, const char * originator_address, int originator_port, void *userdata);
ssh_channel channel_open_request_auth_agent_callback(ssh_session session, void *userdata);

extern const struct ssh_callbacks_struct callbacks;

#endif
