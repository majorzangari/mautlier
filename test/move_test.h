#ifndef MAUTLIER_MOVE_TEST_H
#define MAUTLIER_MOVE_TEST_H

#include "board.h"
#include "diagnostic_tools.h"
#include "fen.h"
#include "move.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// tests all available moves at the give position to make sure board is
// equivalent after making and unmaking the move
bool test_move_making(char *pos) {
  Board *board = fen_to_board(pos);
  Move moves[MAX_MOVES];

  int moves_number = generate_moves(board, moves);
  for (int i = 0; i < moves_number; i++) {
    Board *board_copy = malloc(sizeof(Board));
    memcpy(board_copy, board, sizeof(Board));

    if (!board_valid(board_copy)) {
      fprintf(stderr, "Invalid board before making move %d: %hu\n", i,
              moves[i]);
      fprintf(stderr, "Board: %s\n", board_to_debug_string(board));
      free(board_copy);
      free(board);
      return false;
    }

    board_make_move(board, moves[i]);
    if (!board_valid(board)) {
      fprintf(stderr, "Invalid board after making move %d: %hu\n", i, moves[i]);
      fprintf(stderr, "Board: %s\n", board_to_debug_string(board));
      free(board_copy);
      free(board);
      return false;
    }
    board_unmake_move(board, moves[i]);

    if (!board_valid(board)) {
      fprintf(stderr, "Invalid board after unmaking move %d: %s\n", i,
              move_to_string(moves[i]));
      free(board_copy);
      free(board);
      return false;
    }

    if (!compare_boards(board, board_copy)) {
      fprintf(stderr, "Move test failed for move %d: %hu\n", i, moves[i]);
      fprintf(stderr, "Board diff: %s\n", last_board_compare_diff());
      free(board_copy);
      free(board);
      return false;
    }
  }

  free(board);
  return true;
}

bool make_unmake_suite() {
  // clang-format off
  char positions[5][256] = {
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",      // simple position
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",      // simple black position
      "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1",            // castling shenanigans
      "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1",            // black castling shenanigans
      "rnbqkbnr/pppp1ppp/4p3/8/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 2", // checkmate available
  };
  // clang-format on
  for (size_t i = 0; i < (sizeof(positions) / sizeof(positions[0])); i++) {
    if (!test_move_making(positions[i])) {
      fprintf(stderr, "Move test failed for position: %s\n", positions[i]);
      return false;
    }
  }

  return true;
}
#endif
