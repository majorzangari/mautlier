#ifndef MAUTLIER_MOVE_TEST_H
#define MAUTLIER_MOVE_TEST_H

#include "board.h"
#include "constants.h"
#include "fen.h"
#include "move.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool test_move_making() {
  init_data();
  char str[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Board *board = fen_to_board(str);
  Move moves[MAX_MOVES];
  printf("%s\n", board_to_string(board));

  int moves_number = generate_moves(board, moves);
  for (int i = 0; i < moves_number; i++) {
    Board *board_copy = malloc(sizeof(Board));
    memcpy(board_copy, board, sizeof(Board));
    board_make_move(board, moves[i]);
    board_unmake_move(board, moves[i]);
    if (memcmp(board, board_copy, sizeof(Board)) != 0) {
      printf("Move making test failed for move %d\n", i);
      printf("Original board:\n%s\n", board_to_debug_string(board_copy));
      printf("After making and unmaking move:\n%s\n",
             board_to_debug_string(board));
      free(board_copy);
      free(board);
      return false;
    }
  }

  free(board);
  return true;
}
#endif
