#include <stdlib.h>
#include <stdio.h>

#include <sys/mman.h>
#include <time.h>

#include <libssh/server.h>

#include "ssh_daemon.h"
#include "callbacks/session.h"
#include "callbacks/channel.h"

static
ssh_client_t * get_available_ssh_client_slot(ssh_db_t * ssh_db) {
  for(unsigned int i = 0; i < MAX_CLIENTS; i++) {
    if (atomic_load(ssh_db->active + i) == false) {
      if(ssh_db->slots[i].pid) {
        int status;
        waitpid(ssh_db->slots[i].pid, &status, 0);
        if(WIFEXITED(status)) {
          printf("[ssh_server]: ssh_client %u exited with %s\n", i, WEXITSTATUS(status) == EXIT_SUCCESS ? "SUCCESS" : "FAILURE");
        } else {
          printf("[ssh_server]: Warning. ssh_client %u terminated outside of a normal exit route.\n", i);
        }
      }
      return ssh_db->slots + i;
    }
  }
  return NULL;
}

static
void reset_ssh_client(ssh_client_t * ssh_client) {
  printf("[ssh_client %u]: reset: params\n", ssh_client->id);
  ssh_client->pre_channel = true;
  ssh_client->authenticated = false;
  ssh_client->auth_attempts = false;
  ssh_client->pleaseKill = false;

  printf("[ssh_client %u]: reset: channel\n", ssh_client->id);
  if(ssh_client->channel) {
    ssh_channel_free(ssh_client->channel);
    ssh_client->channel = NULL;
  }

  printf("[ssh_client %u]: reset: session\n", ssh_client->id);
  if(ssh_client->session) {
    ssh_event_remove_session(ssh_client->event, ssh_client->session);
    ssh_disconnect(ssh_client->session);
    ssh_free(ssh_client->session);
    ssh_client->session = NULL;
  }

  printf("[ssh_client %u]: reset: event\n", ssh_client->id);
  if(ssh_client->event) {
    ssh_event_free(ssh_client->event);
    ssh_client->event = NULL;
  }

  printf("[ssh_client %u]: reset: active\n", ssh_client->id);
  atomic_store(ssh_client->active, false);

  printf("[ssh_client %u]: Completed shutdown successfully\n", ssh_client->id);
}

static
int ssh_client_task(ssh_client_t * ssh_client) {
  ssh_client->event = ssh_event_new();
  if (!ssh_client->event) {
    printf("[ssh_client %u]: Could not create polling context\n", ssh_client->id);
    printf("!!![ssh_client %u]: EXIT A\n", ssh_client->id);
    reset_ssh_client(ssh_client);
    return EXIT_FAILURE;
  }

  ssh_set_server_callbacks(ssh_client->session, &(ssh_client->ssh_client_session_callbacks));

  if (ssh_handle_key_exchange(ssh_client->session) != SSH_OK) {
      printf("[ssh_client %u]: %s\n", ssh_client->id, ssh_get_error(ssh_client->session));
      printf("!!![ssh_client %u]: EXIT B\n", ssh_client->id);
      reset_ssh_client(ssh_client);
      return EXIT_FAILURE;
  }

  ssh_event_add_session(ssh_client->event, ssh_client->session);

  printf("[ssh_client %u]: Completed setup successfully\n", ssh_client->id);

  do
  {
    if (ssh_event_dopoll(ssh_client->event, -1) == SSH_ERROR) {
      printf("[ssh_client %u]: Closed. Error: %s\n", ssh_client->id, ssh_get_error(ssh_client->session));
      printf("!!![ssh_client %u]: EXIT C\n", ssh_client->id);
      reset_ssh_client(ssh_client);
      return EXIT_FAILURE;
    }
  } while(
    (ssh_channel_is_open(ssh_client->channel) || ssh_client->pre_channel)
    && !ssh_client->pleaseKill
    && (ssh_client->authenticated || ssh_client->auth_attempts < MAX_AUTH_ATTEMPTS));

  printf("!!![ssh_client %u]: EXIT D\n", ssh_client->id);
  reset_ssh_client(ssh_client);

  return EXIT_SUCCESS;
}

void ssh_daemon(void) {
  // signal(SIGCHLD, SIG_IGN);

  ssh_db_t * ssh_db = (ssh_db_t *) mmap(NULL, sizeof(ssh_db_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  for(size_t i = 0; i < MAX_CLIENTS; i++) {
    ssh_client_t * this_slot = ssh_db->slots + i;
    *this_slot = (ssh_client_t) {
      .id = i,
      .pid = 0,
      .active = ssh_db->active + i,
      .pleaseKill = false,

      .pre_channel = true,
      .authenticated = false,
      .auth_attempts = false,

      .session = NULL,
      .ssh_client_session_callbacks = session_callbacks,
      .channel = NULL,
      .ssh_client_channel_callbacks = channel_callbacks,

      .event = NULL,
    };

    atomic_store(this_slot->active, false);

    this_slot->ssh_client_session_callbacks.userdata = (void *) this_slot;
    ssh_callbacks_init(&(this_slot->ssh_client_session_callbacks));

    ssh_db->slots[i].ssh_client_channel_callbacks.userdata = (void *) this_slot;
    ssh_callbacks_init(&(this_slot->ssh_client_channel_callbacks));
  }

  ssh_bind bind;

  if (ssh_init() < 0) {
      printf("ssh_init failed\n");
      return;
  }

  bind = ssh_bind_new();
  if (bind == NULL) {
      printf("ssh_bind_new failed\n");
      return;
  }

  const int log_level = SSH_LOG_WARNING;
  if(
    ssh_bind_options_set(bind, SSH_BIND_OPTIONS_LOG_VERBOSITY, &log_level)
    || ssh_bind_options_set(bind, SSH_BIND_OPTIONS_BINDADDR, BINDADDR)
    || ssh_bind_options_set(bind, SSH_BIND_OPTIONS_BINDPORT_STR, BINDPORT)
    || ssh_bind_options_set(bind, SSH_BIND_OPTIONS_RSAKEY,   KEYS_FOLDER "rsa-key")
    || ssh_bind_options_set(bind, SSH_BIND_OPTIONS_ECDSAKEY, KEYS_FOLDER "ecdsa-key")
    || ssh_bind_options_set(bind, SSH_BIND_OPTIONS_BANNER, "Welcome to spaceGame")
  ) {
    printf("ssh_daemon had an issue setting the configuration preferences\n");
    return;
  }

  if(ssh_bind_listen(bind) < 0) {
      printf("Fatal. Unable to start the spaceGame server. Error: ");
      printf("%s\n", ssh_get_error(bind));
      ssh_bind_free(bind);
      return;
  }


  const struct timespec retry_after_ts = (struct timespec) {
    .tv_sec = RETRY_AFTER,
    .tv_nsec = 0L
  };

  for(;;) {
    ssh_client_t * ssh_client = get_available_ssh_client_slot(ssh_db);

    if(!ssh_client) {
      printf("no ssh_clients available to connect to, try again later\n");
      nanosleep(&retry_after_ts, NULL);
      continue;
    }

    ssh_client->session = ssh_new();
    if (!ssh_client->session) {
      printf("[ssh_client %u]: Failed to allocate session\n", ssh_client->id);
      continue;
    }


    if(ssh_bind_accept(bind, ssh_client->session) != SSH_ERROR) {
      atomic_store(ssh_client->active, true);
      printf("[ssh_client %u]: Spawning\n", ssh_client->id);
      int pid = fork();
      if(!pid) {
        //child
        ssh_bind_free(bind);
        exit(
          ssh_client_task(ssh_client)
        );
      } else {
        ssh_client->pid = pid;
      }
    }
  }

  ssh_bind_free(bind);
  ssh_finalize();
  munmap(ssh_db, sizeof(ssh_db_t));
}
