#include <stdio.h>

#include "ssh.h"
#include "../ssh_client.h"

//ssh_auth_callback
/**
 * @brief SSH authentication callback for password and publickey auth.
 *
 * @param prompt        Prompt to be displayed.
 * @param buf           Buffer to save the password. You should null-terminate it.
 * @param len           Length of the buffer.
 * @param echo          Enable or disable the echo of what you type.
 * @param verify        Should the password be verified?
 * @param userdata      Userdata to be passed to the callback function. Useful
 *                      for GUI applications.
 *
 * @return              0 on success, < 0 on error.
 */
int auth_callback(const char *prompt, char *buf, size_t len, int echo, int verify, void *userdata) {
  ssh_client_t * ssh_ssh_client = (ssh_client_t *) userdata;
  printf("!!![ssh_client %u]: %s\n", ssh_ssh_client->id, "auth_callback");

  return -1;
}

//ssh_log_callback
/**
 * @brief SSH log callback. All logging messages will go through this callback
 * @param session Current session handler
 * @param priority Priority of the log, the smaller being the more important
 * @param message the actual message
 * @param userdata Userdata to be passed to the callback function.
 */
void log_callback(ssh_session session, int priority, const char *message, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("!!![ssh_client %u]: %s\n", ssh_client->id, "log_callback");
}

//ssh_status_callback
/**
 * @brief SSH Connection status callback.
 * @param session Current session handler
 * @param status Percentage of connection status, going from 0.0 to 1.0
 * once connection is done.
 * @param userdata Userdata to be passed to the callback function.
 */
void status_callback(void *userdata, float status) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("!!![ssh_client %u]: %s\n", ssh_client->id, "status_callback");
  printf("!!!\t[ssh_client %u]: %f\n", ssh_client->id, status);
}

//ssh_global_request_callback
/**
 * @brief SSH global request callback. All global request will go through this
 * callback.
 * @param session Current session handler
 * @param message the actual message
 * @param userdata Userdata to be passed to the callback function.
 */
void global_request_callback(ssh_session session, ssh_message message, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("!!![ssh_client %u]: %s\n", ssh_client->id, "global_request_callback");
}

//ssh_channel_open_request_x11_callback
/**
 * @brief Handles an SSH new channel open X11 request. This happens when the server
 * sends back an X11 connection attempt. This is a ssh_client-side API
 * @param session current session handler
 * @param userdata Userdata to be passed to the callback function.
 * @returns a valid ssh_channel handle if the request is to be allowed
 * @returns NULL if the request should not be allowed
 * @warning The channel pointer returned by this callback must be closed by the application.
 */
ssh_channel channel_open_request_x11_callback(ssh_session session, const char * originator_address, int originator_port, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("!!![ssh_client %u]: %s\n", ssh_client->id, "channel_open_request_x11_callback");

  return NULL;
}

//ssh_channel_open_request_auth_agent_callback
/**
 * @brief Handles an SSH new channel open "auth-agent" request. This happens when the server
 * sends back an "auth-agent" connection attempt. This is a ssh_client-side API
 * @param session current session handler
 * @param userdata Userdata to be passed to the callback function.
 * @returns a valid ssh_channel handle if the request is to be allowed
 * @returns NULL if the request should not be allowed
 * @warning The channel pointer returned by this callback must be closed by the application.
 */
ssh_channel channel_open_request_auth_agent_callback(ssh_session session, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("!!![ssh_client %u]: %s\n", ssh_client->id, "channel_open_request_auth_agent_callback");

  return NULL;
}

const struct ssh_callbacks_struct callbacks = {
  /**
   * This functions will be called if e.g. a keyphrase is needed.
   */
  .auth_function = auth_callback,
  /**
   * This function will be called each time a loggable event happens.
   */
  .log_function = log_callback,
  /**
   * This function gets called during connection time to indicate the
   * percentage of connection steps completed.
   */
  .connect_status_function = status_callback,
  /**
   * This function will be called each time a global request is received.
   */
  .global_request_function = global_request_callback,
  /**
   * This function will be called when an incoming X11 request is received.
   */
  .channel_open_request_x11_function = channel_open_request_x11_callback,
  /**
   * This function will be called when an incoming "auth-agent" request is received.
   */
  .channel_open_request_auth_agent_function = channel_open_request_auth_agent_callback
};
