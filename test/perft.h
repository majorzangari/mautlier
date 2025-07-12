#include "board.h"
#include "move.h"

#include <stdint.h>

uint64_t perft(Board *board, int depth) {
  if (depth == 0) {
    return 1;
  }

  uint64_t total_nodes = 0;
  Move moves[MAX_MOVES];
  // TODO: finish
}
