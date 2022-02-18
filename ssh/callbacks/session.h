#ifndef CALLBACKS_SESSION_H
#define CALLBACKS_SESSION_H

#include <libssh/callbacks.h>

int auth_password_callback(ssh_session session, const char *user, const char *password, void *userdata);
int auth_none_callback(ssh_session session, const char *user, void *userdata);
int auth_gssapi_mic_callback(ssh_session session, const char *user, const char *principal, void *userdata);
int auth_pubkey_callback(ssh_session session, const char *user, struct ssh_key_struct *pubkey, char signature_state, void *userdata);
int service_request_callback(ssh_session session, const char *service, void *userdata);
ssh_channel channel_open_request_session_callback(ssh_session session, void *userdata);
ssh_string gssapi_select_oid_callback(ssh_session session, const char *user, int n_oid, ssh_string *oids, void *userdata);
int gssapi_accept_sec_ctx_callback(ssh_session session, ssh_string input_token, ssh_string *output_token, void *userdata);
int gssapi_verify_mic_callback(ssh_session session, ssh_string mic, void *mic_buffer, size_t mic_buffer_size, void *userdata);

extern const struct ssh_server_callbacks_struct session_callbacks;

#endif
