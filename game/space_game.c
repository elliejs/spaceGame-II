#include <stdlib.h>
#include <locale.h>

#include "../ssh/ssh_daemon.h"

int main() {
  setlocale(LC_ALL, "");
  ssh_daemon();
  return EXIT_FAILURE;
}
