#include "state/board.h"
#include "state/constants.h"
#include "state/fen.h"
#include "state/move.h"

#include <stdio.h>

int main(void) {
  init_data();

  char str[] = "rnbqkbnr/8/8/8/8/8/8/R222R w KQkq - 0 1";
  Board *board = fen_to_board(str);
  printf("%s\n", board_to_string(board));
  Move moves[MAX_MOVES];
  generate_moves(board, moves);
  for (int i = 0; i < MAX_MOVES && moves[i] != 0; i++) {
    int from = moves[i] >> 10;
    int to = (moves[i] & 0x03FF) >> 4;
    int flags = (moves[i] & 0x000F);
    printf("Move %d: from %d to %d flags %d\n", i, from, to, flags);
  }
  return 0;
}
