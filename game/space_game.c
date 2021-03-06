#include <stdlib.h>
#include <locale.h>

#include "../ssh/ssh_daemon.h"
#include "../world/world_server.h"

int main(int argc, char const * argv[]) {
  setlocale(LC_ALL, "");
  start_world_server();
  ssh_daemon(argc == 2 ? argv[1] : NULL);
  return EXIT_FAILURE;
}
