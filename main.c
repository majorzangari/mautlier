#include "cli.h"
#include "constants.h"
#include "debug_printer.h"
#include "hash.h"
#include "misc.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  // setup
  srand(0); // TODO: set to time
  init_data();
  init_zobrist();

  for (int i = 0; i < argc; i++) {

    if (strcmp(argv[i], "--mode") == 0) {
      if (i + 1 < argc) {
        if (or_strcmp(argv[i + 1], 3, "cli", "console", "c")) {
          return cli_main();
        } else if (or_strcmp(argv[i + 1], 2, "test", "t")) {
          // do test thing
        }
      } else {
        printf("Error: --mode requires an argument\n");
      }

    } else if (strcmp(argv[i], "--flag") == 0) {
      if (i + 1 < argc) {
        DP_ADD_FLAG(argv[i]);
      } else {
        printf("Error: --flag requires an argument\n");
      }
    }
  }

  return 0;
}
