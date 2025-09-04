#ifndef MAUTLIER_UCI_H
#define MAUTLIER_UCI_H

#include <stdbool.h>
#define UCI_EXIT_UNKNOWN 2

#define ID_NAME "mautlier"
#define ID_VERSION "0.1"
#define ID_AUTHOR "major zangari"

typedef struct {
  bool transposition_table;
  bool quiescence_search;
  bool book;
  bool null_move_pruning;
  bool move_ordering;
  bool lmr;
} UCIOptions;

int uci_main();

#endif
