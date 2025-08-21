#include "benchmark.h"
#include "board.h"
#include "perft.h"
#include <time.h>

double moves_per_second(int depth) {
  Board *board = init_default_board();
  clock_t start = clock();
  uint64_t nodes = perft(board, depth);
  clock_t end = clock();
  double seconds = (double)(end - start) / CLOCKS_PER_SEC;
  return nodes / seconds;
}
