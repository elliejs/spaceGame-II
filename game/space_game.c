#include <stdlib.h>
#include <locale.h>

#include "../ssh/ssh_daemon.h"
#include "../world_db/world_db.h"

int main() {
  setlocale(LC_ALL, "");
  start_world_db();
  ssh_daemon();
  return EXIT_FAILURE;
}
