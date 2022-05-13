#include <stdlib.h>
#include <locale.h>

#include "../ssh/ssh_daemon.h"
#include "../world/world_server.h"

int main() {
  setlocale(LC_ALL, "");
  start_world_server();
  ssh_daemon();
  return EXIT_FAILURE;
}
