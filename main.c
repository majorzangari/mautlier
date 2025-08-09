#include "board.h"
#include "constants.h"
#include "debug_printer.h"
#include "diagnostic_tools.h"
#include "fen.h"
#include "hash.h"
#include "move.h"
#include "search.h"
#include "test/move_test.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  srand(0); // TODO: set to time
  init_data();
  init_zobrist();
  for (int i = 0; i < argc; i++) {
    DP_ADD_FLAG(argv[i]);
  }

  if (!make_unmake_suite()) {
    printf("fuck\n");
    return 0;
  }

  char str[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Board *board = fen_to_board(str);
  Move moves[MAX_MOVES];

  while (1) {
    getchar();
    Move selected_move = lazy_search(board, 4);
    board_make_move(board, selected_move);

    printf("%s\n", board_to_string(board));
  }
  free(board);
  return 0;
}
