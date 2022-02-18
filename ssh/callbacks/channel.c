#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "channel.h"
#include "../ssh_client.h"

// ssh_channel_data_callback
/**
 * @brief SSH channel data callback. Called when data is available on a channel
 * @param session Current session handler
 * @param channel the actual channel
 * @param data the data that has been read on the channel
 * @param len the length of the data
 * @param is_stderr is 0 for stdout or 1 for stderr
 * @param userdata Userdata to be passed to the callback function.
 * @returns number of bytes processed by the callee. The remaining bytes will
 * be sent in the next callback message, when more data is available.
 */
int channel_data_callback(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  unsigned char * char_data = (unsigned char *) data;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_data_callback");
  printf("\t[ssh_client %u]: len: %d\n", ssh_client->id, len);
  printf("\t[ssh_client %u]: data: ", ssh_client->id);
  for(int i = 0; i < len; i++) {
    printf("%x ", char_data[i]);
  }
  printf("\n");

  unsigned char control_char = (char) 0x00;
  if(len == 3 && char_data[0] == 0x1b && char_data[1] == 0x5b) control_char = char_data[2]; //arrows
  else if (len == 1) control_char = char_data[0];

  switch (control_char) {
    case 0x71:  //q
    case 0x51:  //Q
    case 0x03:  //^C
    case 0x1b:  //ESC
      ssh_client->pleaseKill = true;
      break;
  }

  return (int) len;
}

// ssh_channel_eof_callback
/**
 * @brief SSH channel eof callback. Called when a channel receives EOF
 * @param session Current session handler
 * @param channel the actual channel
 * @param userdata Userdata to be passed to the callback function.
 */
void channel_eof_callback(ssh_session session, ssh_channel channel, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_eof_callback");
}

// ssh_channel_close_callback
/**
 * @brief SSH channel close callback. Called when a channel is closed by remote peer
 * @param session Current session handler
 * @param channel the actual channel
 * @param userdata Userdata to be passed to the callback function.
 */
void channel_close_callback(ssh_session session, ssh_channel channel, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_close_callback");
  ssh_client->pleaseKill = true;
}

// ssh_channel_signal_callback
/**
 * @brief SSH channel signal callback. Called when a channel has received a signal
 * @param session Current session handler
 * @param channel the actual channel
 * @param signal the signal name (without the SIG prefix)
 * @param userdata Userdata to be passed to the callback function.
 */
void channel_signal_callback(ssh_session session, ssh_channel channel, const char *signal, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_signal_callback");
  printf("\t[ssh_client %u]: SIGNAL %s\n", ssh_client->id, signal);
  ssh_client->pleaseKill = true;
}

// ssh_channel_exit_status_callback
/**
 * @brief SSH channel exit status callback. Called when a channel has received an exit status
 * @param session Current session handler
 * @param channel the actual channel
 * @param userdata Userdata to be passed to the callback function.
 */
void channel_exit_status_callback(ssh_session session, ssh_channel channel, int exit_status, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_exit_status_callback");
  printf("\t[ssh_client %u]: exit_status: %d\n", ssh_client->id, exit_status);
  ssh_client->pleaseKill = true;
}

// ssh_channel_exit_signal_callback
/**
 * @brief SSH channel exit signal callback. Called when a channel has received an exit signal
 * @param session Current session handler
 * @param channel the actual channel
 * @param signal the signal name (without the SIG prefix)
 * @param core a boolean telling wether a core has been dumped or not
 * @param errmsg the description of the exception
 * @param lang the language of the description (format: RFC 3066)
 * @param userdata Userdata to be passed to the callback function.
 */
void channel_exit_signal_callback(ssh_session session, ssh_channel channel, const char *signal, int core, const char *errmsg, const char *lang, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_exit_signal_callback");
  ssh_client->pleaseKill = true;
}

// ssh_channel_pty_request_callback
/**
 * @brief SSH channel PTY request from a ssh_client.
 * @param channel the channel
 * @param term The type of terminal emulation
 * @param width width of the terminal, in characters
 * @param height height of the terminal, in characters
 * @param pxwidth width of the terminal, in pixels
 * @param pxheight height of the terminal, in pixels
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the pty request is accepted
 * @returns -1 if the request is denied
 */
int channel_pty_request_callback(ssh_session session, ssh_channel channel, const char *term, int width, int height, int pxwidth, int pxheight, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_pty_request_callback");

  printf("\t[ssh_client %u]: term: %s\n", ssh_client->id, term);
  printf("\t[ssh_client %u]: width: %d\n", ssh_client->id, width);
  printf("\t[ssh_client %u]: height: %d\n", ssh_client->id, height);
  printf("\t[ssh_client %u]: pxwidth: %d\n", ssh_client->id, pxwidth);
  printf("\t[ssh_client %u]: pxheight: %d\n", ssh_client->id, pxheight);

  printf("\t[ssh_client %u]: pty request successful.\n", ssh_client->id);

  return 0;
}

// ssh_channel_shell_request_callback
/**
 * @brief SSH channel Shell request from a ssh_client.
 * @param channel the channel
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the shell request is accepted
 * @returns 1 if the request is denied
 */
int channel_shell_request_callback(ssh_session session, ssh_channel channel, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_shell_request_callback");
  printf("\t[ssh_client %u]: shell request successful\n", ssh_client->id);

  return 0;
}

// ssh_channel_auth_agent_req_callback
/**
 * @brief SSH auth-agent-request from the ssh_client. This request is
 * sent by a ssh_client when agent forwarding is available.
 * Server is free to ignore this callback, no answer is expected.
 * @param channel the channel
 * @param userdata Userdata to be passed to the callback function.
 */
void channel_auth_agent_req_callback(ssh_session session, ssh_channel channel, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_auth_agent_req_callback");
}

// ssh_channel_x11_req_callback
/**
 * @brief SSH X11 request from the ssh_client. This request is
 * sent by a ssh_client when X11 forwarding is requested(and available).
 * Server is free to ignore this callback, no answer is expected.
 * @param channel the channel
 * @param userdata Userdata to be passed to the callback function.
 */
void channel_x11_req_callback(ssh_session session, ssh_channel channel, int single_connection, const char *auth_protocol, const char *auth_cookie, uint32_t screen_number, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_x11_req_callback");
}

// ssh_channel_pty_window_change_callback
/**
 * @brief SSH channel PTY windows change (terminal size) from a ssh_client.
 * @param channel the channel
 * @param width width of the terminal, in characters
 * @param height height of the terminal, in characters
 * @param pxwidth width of the terminal, in pixels
 * @param pxheight height of the terminal, in pixels
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the pty request is accepted
 * @returns -1 if the request is denied
 */
int channel_pty_window_change_callback(ssh_session session, ssh_channel channel, int width, int height, int pxwidth, int pxheight, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_pty_window_change_callback");

  printf("\t[ssh_client %u]: width: %d\n", ssh_client->id, width);
  printf("\t[ssh_client %u]: height: %d\n", ssh_client->id, height);
  printf("\t[ssh_client %u]: pxwidth: %d\n", ssh_client->id, pxwidth);
  printf("\t[ssh_client %u]: pxheight: %d\n", ssh_client->id, pxheight);
  return -1;
}

// ssh_channel_exec_request_callback
/**
 * @brief SSH channel Exec request from a ssh_client.
 * @param channel the channel
 * @param command the shell command to be executed
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the exec request is accepted
 * @returns 1 if the request is denied
 */
int channel_exec_request_callback(ssh_session session, ssh_channel channel, const char *command, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_exec_request_callback");

  return 1;
}

// ssh_channel_env_request_callback
/**
 * @brief SSH channel environment request from a ssh_client.
 * @param channel the channel
 * @param env_name name of the environment value to be set
 * @param env_value value of the environment value to be set
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the env request is accepted
 * @returns 1 if the request is denied
 * @warning some environment variables can be dangerous if changed (e.g.
 * 			LD_PRELOAD) and should not be fulfilled.
 */
int channel_env_request_callback(ssh_session session, ssh_channel channel, const char *env_name, const char *env_value, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_env_request_callback");

  printf("\t[ssh_client %u]: env_name: %s\n\t[ssh_client %u]: env_value: %s\n",
          ssh_client->id, env_name,
          ssh_client->id, env_value);
  return 1;
}

// ssh_channel_subsystem_request_callback
/**
 * @brief SSH channel subsystem request from a ssh_client.
 * @param channel the channel
 * @param subsystem the subsystem required
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the subsystem request is accepted
 * @returns 1 if the request is denied
 */
int channel_subsystem_request_callback(ssh_session session, ssh_channel channel, const char *subsystem, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_subsystem_request_callback");

  return 1;
}

// ssh_channel_write_wontblock_callback
/**
 * @brief SSH channel write will not block (flow control).
 *
 * @param channel the channel
 *
 * @param[in] bytes size of the remote window in bytes. Writing as much data
 *            will not block.
 *
 * @param[in] userdata Userdata to be passed to the callback function.
 *
 * @returns 0 default return value (other return codes may be added in future).
 */
int channel_write_wontblock_callback(ssh_session session, ssh_channel channel, size_t bytes, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_write_wontblock_callback");
  printf("\t[ssh_client %u]: bytes: %lu\n", ssh_client->id, bytes);

  return 0;
}

const struct ssh_channel_callbacks_struct channel_callbacks = {
  /**
  * This functions will be called when there is data available.
  */
  .channel_data_function = channel_data_callback,
  /**
  * This functions will be called when the channel has received an EOF.
  */
  .channel_eof_function = channel_eof_callback,
  /**
  * This functions will be called when the channel has been closed by remote
  */
  .channel_close_function = channel_close_callback,
  /**
  * This functions will be called when a signal has been received
  */
  .channel_signal_function = channel_signal_callback,
  /**
  * This functions will be called when an exit status has been received
  */
  .channel_exit_status_function = channel_exit_status_callback,
  /**
  * This functions will be called when an exit signal has been received
  */
  .channel_exit_signal_function = channel_exit_signal_callback,
  /**
  * This function will be called when a ssh_client requests a PTY
  */
  .channel_pty_request_function = channel_pty_request_callback,
  /**
  * This function will be called when a ssh_client requests a shell
  */
  .channel_shell_request_function = channel_shell_request_callback,
  /** This function will be called when a ssh_client requests agent
  * authentication forwarding.
  */
  .channel_auth_agent_req_function = channel_auth_agent_req_callback,
  /** This function will be called when a ssh_client requests X11
  * forwarding.
  */
  .channel_x11_req_function = channel_x11_req_callback,
  /** This function will be called when a ssh_client requests a
  * window change.
  */
  .channel_pty_window_change_function = channel_pty_window_change_callback,
  /** This function will be called when a ssh_client requests a
  * command execution.
  */
  .channel_exec_request_function = channel_exec_request_callback,
  /** This function will be called when a ssh_client requests an environment
  * variable to be set.
  */
  .channel_env_request_function = channel_env_request_callback,
  /** This function will be called when a ssh_client requests a subsystem
  * (like sftp).
  */
  .channel_subsystem_request_function = channel_subsystem_request_callback,
  /** This function will be called when the channel write is guaranteed
  * not to block.
  */
  .channel_write_wontblock_function = channel_write_wontblock_callback
};
