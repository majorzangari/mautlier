#include "state/board.h"
#include "state/constants.h"
#include "state/fen.h"
#include "state/move.h"
#include "test/move_test.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
  srand(0); // TODO: set to time
  test_move_making();
  // init_data();
  // char str[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  // Board *board = fen_to_board(str);
  // Move moves[MAX_MOVES];
  // printf("%s\n", board_to_string(board));
  //
  // while (1) {
  //   int moves_number = generate_moves(board, moves);
  //   printf("%d moves generated\n", moves_number);
  //   getchar();
  //   board_make_move(board, moves[0]);
  //
  //   printf("%s\n", board_to_string(board));
  // }
  // free(board);
  // return 0;
}
