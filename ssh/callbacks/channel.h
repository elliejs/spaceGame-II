#ifndef CALLBACKS_CHANNEL_H
#define CALLBACKS_CHANNEL_H

#include <libssh/callbacks.h>

int channel_data_callback(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata);
void channel_eof_callback(ssh_session session, ssh_channel channel, void *userdata);
void channel_close_callback(ssh_session session, ssh_channel channel, void *userdata);
void channel_signal_callback(ssh_session session, ssh_channel channel, const char *signal, void *userdata);
void channel_exit_status_callback(ssh_session session, ssh_channel channel, int exit_status, void *userdata);
void channel_exit_signal_callback(ssh_session session, ssh_channel channel, const char *signal, int core, const char *errmsg, const char *lang, void *userdata);
int channel_pty_request_callback(ssh_session session, ssh_channel channel, const char *term, int width, int height, int pxwidth, int pwheight, void *userdata);
int channel_shell_request_callback(ssh_session session, ssh_channel channel, void *userdata);
void channel_auth_agent_req_callback(ssh_session session, ssh_channel channel, void *userdata);
void channel_x11_req_callback(ssh_session session, ssh_channel channel, int single_connection, const char *auth_protocol, const char *auth_cookie, uint32_t screen_number, void *userdata);
int channel_pty_window_change_callback(ssh_session session, ssh_channel channel, int width, int height, int pxwidth, int pwheight, void *userdata);
int channel_exec_request_callback(ssh_session session, ssh_channel channel, const char *command, void *userdata);
int channel_env_request_callback(ssh_session session, ssh_channel channel, const char *env_name, const char *env_value, void *userdata);
int channel_subsystem_request_callback(ssh_session session, ssh_channel channel, const char *subsystem, void *userdata);
int channel_write_wontblock_callback(ssh_session session, ssh_channel channel, size_t bytes, void *userdata);

extern const struct ssh_channel_callbacks_struct channel_callbacks;

#endif
