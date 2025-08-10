#ifndef MAUTLIER_MOVE_TEST_H
#define MAUTLIER_MOVE_TEST_H

#include "board.h"
#include "diagnostic_tools.h"
#include "fen.h"
#include "move.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// TODO: fix exit shenanigans

void check_invalid(Board *board, const char *context, ...) { // TODO: fix hacks
  if (!board_valid(board)) {
    va_list args;
    va_start(args, context);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), context, args);

    fprintf(stderr, "Invalid board detected: %s\n", buffer);
    fprintf(stderr, "Board: %s\n", board_to_debug_string(board));
    exit(1);
  }
}

void check_compare(Board *copy, Board *changed, const char *context, ...) {
  if (!compare_boards(copy, changed)) {
    va_list args;
    va_start(args, context);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), context, args);

    fprintf(stderr, "Board comparison failed: %s\n", buffer);
    fprintf(stderr, "Board diff: %s\n", last_board_compare_diff());
    fprintf(stderr, "%s\n", board_to_string(changed));
    // fprintf(stderr, "%s\n", board_to_debug_string(changed));
    exit(1);
  }
}

// tests all available moves at the give position to make sure board is
// equivalent after making and unmaking the move
void test_move_making(char *pos) {
  Board *board = fen_to_board(pos);
  Move moves[MAX_MOVES];

  int moves_number = generate_moves(board, moves);
  for (int i = 0; i < moves_number; i++) {
    Board *board_copy = malloc(sizeof(Board));
    memcpy(board_copy, board, sizeof(Board));

    check_invalid(board, "before making move %s", move_to_string(moves[i]));

    board_make_move(board, moves[i]);

    check_invalid(board, "after making move %s", move_to_string(moves[i]));

    board_unmake_move(board, moves[i]);
    check_invalid(board, "after unmaking move %s", move_to_string(moves[i]));

    check_compare(board_copy, board, "after unmaking move %s",
                  move_to_string(moves[i]));
    free(board_copy);
  }

  free(board);
}

bool make_unmake_suite() {
  // clang-format off
  char positions[][256] = {
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",      // simple position
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",      // simple black position
      "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1",            // castling shenanigans
      "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1",            // black castling shenanigans
      "rnbqkbnr/pppp1ppp/4p3/8/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 2", // checkmate available
      "r1bqkb1r/2p1ppp1/p1p5/3p4/4P3/2PP1Q2/P1P2PPP/R1B1K1NR b KkQq - 1 0",
      "r1bqkb1r/2p1ppp1/p1p5/3p4/4P3/2PP1Q2/P1P2PPP/R1B1K1NR b KkQq - 1 0",
      "r1bqkb1r/2p1ppp1/p1p5/3p4/4PQ2/2PP4/PBP2PpP/R3K1NR b KQkq - 1 2"
  };
  // clang-format on
  for (size_t i = 0; i < (sizeof(positions) / sizeof(positions[0])); i++) {
    test_move_making(positions[i]);
  }

  return true;
}
#endif
