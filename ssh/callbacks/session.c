#include <stdio.h>
#include <string.h>

#include "session.h"
#include "../ssh_client.h"


#include "../../astrogammon/astrogammon_client.h"
#include "../../astrogammon/astrogammon_server.h"
#include "../../users/user_db.h"

// ssh_auth_password_callback
/**
 * @brief SSH authentication callback.
 * @param session Current session handler
 * @param user User that wants to authenticate
 * @param password Password used for authentication
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 */
int auth_password_callback(ssh_session session, const char *user, const char *password, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "auth_password_callback");

  printf("\t[ssh_client %u]: user: %s\n\t[ssh_client %u]: pass: %s\n",
          ssh_client->id, user,
          ssh_client->id, password);

  if (!strcmp(user, "astrogammon")) {
    ssh_client->authenticated = true;
    ssh_client->mode = ASTROGAMMON;
    if (strcmp(password, "")) {
      printf("joining existing game\n");
      join_game((char *) password);
    } else {
      printf("creating new astrogammon game\n");
      join_game(create_game(create_ruleset_classic()));
    }
    printf("done\n");
    return SSH_AUTH_SUCCESS;
  }

  off_t user_index = login(user, password);
  printf("\t[ssh_client %u]: user_index: %lld\n", ssh_client->id, user_index);
  if(user_index >= 0) {
    ssh_client->authenticated = true;
    ssh_client->user_index = user_index;
    ssh_client->mode = SPACEGAME;
    return SSH_AUTH_SUCCESS;
  } else {
    ssh_client->auth_attempts++;
    return SSH_AUTH_DENIED;
  }
}

// ssh_auth_none_callback
/**
 * @brief SSH authentication callback. Tries to authenticates user with the "none" method
 * which is anonymous or passwordless.
 * @param session Current session handler
 * @param user User that wants to authenticate
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 */
 int auth_none_callback(ssh_session session, const char *user, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "auth_none_callback");

  printf("\t[ssh_client %u]: %s\n", ssh_client->id, user);
  return SSH_AUTH_DENIED;
}

// ssh_auth_gssapi_mic_callback
/**
 * @brief SSH authentication callback. Tries to authenticates user with the "gssapi-with-mic" method
 * @param session Current session handler
 * @param user Username of the user (can be spoofed)
 * @param principal Authenticated principal of the user, including realm.
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 * @warning Implementations should verify that parameter user matches in some way the principal.
 * user and principal can be different. Only the latter is guaranteed to be safe.
 */
 int auth_gssapi_mic_callback(ssh_session session, const char *user, const char *principal, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "auth_gssapi_mic_callback");

  return SSH_AUTH_DENIED;
}

// ssh_auth_pubkey_callback
/**
 * @brief SSH authentication callback.
 * @param session Current session handler
 * @param user User that wants to authenticate
 * @param pubkey public key used for authentication
 * @param signature_state SSH_PUBLICKEY_STATE_NONE if the key is not signed (simple public key probe),
 * 							SSH_PUBLICKEY_STATE_VALID if the signature is valid. Others values should be
 * 							replied with a SSH_AUTH_DENIED.
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 */
int auth_pubkey_callback(ssh_session session, const char *user, struct ssh_key_struct *pubkey, char signature_state, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "auth_pubkey_callback");
  printf("[ssh_client %u]: user: %s\n", ssh_client->id, user);
  printf("[ssh_client %u]: signature_state: %s\n", ssh_client->id, signature_state == SSH_PUBLICKEY_STATE_NONE ? "SSH_PUBLICKEY_STATE_NONE" : (signature_state == SSH_PUBLICKEY_STATE_VALID ? "SSH_PUBLICKEY_STATE_VALID" : "BAD_KEY"));
  // printf("\t[ssh_client %u keyinfo]: type: %s\n", ssh_client->id, pubkey->type_c);
  // printf("\t[ssh_client %u keyinfo]:")
  // printf("\t[ssh_client %u keyinfo]:")
  // printf("\t[ssh_client %u keyinfo]:")

  return SSH_AUTH_DENIED;
}

// ssh_service_request_callback
/**
 * @brief Handles an SSH service request
 * @param session current session handler
 * @param service name of the service (e.g. "ssh-userauth") requested
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the request is to be allowed
 * @returns -1 if the request should not be allowed
 */
int service_request_callback(ssh_session session, const char *service, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "service_request_callback");

  printf("\t[ssh_client %u]: %s\n", ssh_client->id,  service);
  if(!strcmp(service, "ssh-userauth")) return SSH_OK;

  return -1;
}

// ssh_channel_open_request_session_callback
/**
 * @brief Handles an SSH new channel open session request
 * @param session current session handler
 * @param userdata Userdata to be passed to the callback function.
 * @returns a valid ssh_channel handle if the request is to be allowed
 * @returns NULL if the request should not be allowed
 * @warning The channel pointer returned by this callback must be closed by the application.
 */
ssh_channel channel_open_request_session_callback(ssh_session session, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_open_request_session_callback");

  ssh_client->channel = ssh_channel_new(ssh_client->session);
  if(!ssh_client->channel) {
    printf("\t[ssh_client %u]: %s\n", ssh_client->id, "Channel could not be created\n");
    ssh_client->pleaseKill = true;
    return NULL;
  }
  ssh_set_channel_callbacks(ssh_client->channel, &(ssh_client->ssh_client_channel_callbacks));
  ssh_client->pre_channel = false;

  ssh_channel_set_blocking(ssh_client->channel, 0);
  printf("\t[ssh_client %u]: %s\n", ssh_client->id, "channel set into nonblocking mode");

  printf("\t[ssh_client %u]: %s\n", ssh_client->id, "channel successfully opened");
  return ssh_client->channel;
}

// ssh_gssapi_select_oid_callback
/*
 * @brief handle the beginning of a GSSAPI authentication, server side.
 * @param session current session handler
 * @param user the username of the ssh_client
 * @param n_oid number of available oids
 * @param oids OIDs provided by the ssh_client
 * @returns an ssh_string containing the chosen OID, that's supported by both
 * ssh_client and server.
 * @warning It is not necessary to fill this callback in if libssh is linked
 * with libgssapi.
 */
ssh_string gssapi_select_oid_callback(ssh_session session, const char *user, int n_oid, ssh_string *oids, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "gssapi_select_oid_callback");

  return NULL;
}

// ssh_gssapi_accept_sec_ctx_callback
/*
 * @brief handle the negociation of a security context, server side.
 * @param session current session handler
 * @param[in] input_token input token provided by ssh_client
 * @param[out] output_token output of the gssapi accept_sec_context method,
 * 				NULL after completion.
 * @returns SSH_OK if the token was generated correctly or accept_sec_context
 * returned GSS_S_COMPLETE
 * @returns SSH_ERROR in case of error
 * @warning It is not necessary to fill this callback in if libssh is linked
 * with libgssapi.
 */
int gssapi_accept_sec_ctx_callback(ssh_session session, ssh_string input_token, ssh_string *output_token, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "gssapi_accept_sec_ctx_callback");

  return SSH_ERROR;
}

//ssh_gssapi_verify_mic_callback
/*
 * @brief Verify and authenticates a MIC, server side.
 * @param session current session handler
 * @param[in] mic input mic to be verified provided by ssh_client
 * @param[in] mic_buffer buffer of data to be signed.
 * @param[in] mic_buffer_size size of mic_buffer
 * @returns SSH_OK if the MIC was authenticated correctly
 * @returns SSH_ERROR in case of error
 * @warning It is not necessary to fill this callback in if libssh is linked
 * with libgssapi.
 */
int gssapi_verify_mic_callback(ssh_session session, ssh_string mic, void *mic_buffer, size_t mic_buffer_size, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "gssapi_verify_mic_callback");

  return SSH_ERROR;
}

const struct ssh_server_callbacks_struct session_callbacks = {
  /** This function gets called when a ssh_client tries to authenticate through
   * password method.
   */
  .auth_password_function = auth_password_callback,

  /** This function gets called when a ssh_client tries to authenticate through
   * none method.
   */
  .auth_none_function = auth_none_callback,

  /** This function gets called when a ssh_client tries to authenticate through
   * gssapi-mic method.
   */
  .auth_gssapi_mic_function = auth_gssapi_mic_callback,

  /** this function gets called when a ssh_client tries to authenticate or offer
   * a public key.
   */
  .auth_pubkey_function = auth_pubkey_callback,

  /** This functions gets called when a service request is issued by the
   * ssh_client
   */
  .service_request_function = service_request_callback,

  /** This functions gets called when a new channel request is issued by
   * the ssh_client
   */
  .channel_open_request_session_function = channel_open_request_session_callback,

  /** This function will be called when a new gssapi authentication is attempted.
   */
  .gssapi_select_oid_function = gssapi_select_oid_callback,

  /** This function will be called when a gssapi token comes in.
   */
  .gssapi_accept_sec_ctx_function = gssapi_accept_sec_ctx_callback,

  /* This function will be called when a MIC needs to be verified.
   */
  .gssapi_verify_mic_function = gssapi_verify_mic_callback
};
