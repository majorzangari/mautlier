#include "perft.h"
#include "diagnostic_tools.h"
#include "misc.h"

#include <stdlib.h>
#include <string.h>

uint64_t perft(Board *board, int depth) {
  if (depth == 0 || board->game_state != GS_ONGOING) {
    return 1ULL;
  }

  uint64_t total_nodes = 0;
  Move moves[MAX_MOVES];
  int num_moves = generate_moves(board, moves);
  if (num_moves == 0) {
    return 1ULL; // checkmate or stalemate
  }

  for (int i = 0; i < num_moves; i++) {
    board_make_move(board, moves[i]);
    if (!king_in_check(board, OPPOSITE_COLOR(board->to_move))) {
      total_nodes += perft(board, depth - 1);
    }
    board_unmake_move(board, moves[i]);
  }

  return total_nodes;
}

// TODO: make nicer this is icky
void perft_divide(Board *board, int depth) {
  if (depth == 0 || board->game_state != GS_ONGOING) {
    return;
  }

  uint64_t total_nodes = 0;
  Move moves[MAX_MOVES];
  int num_moves = generate_moves(board, moves);
  order_alphabetically(moves, num_moves);

  int move_num = 0;
  for (int i = 0; i < num_moves; i++) {
    board_make_move(board, moves[i]);
    if (!king_in_check(board, OPPOSITE_COLOR(board->to_move))) {
      uint64_t nodes = perft(board, depth - 1);
      printf("%d: %s: %lu nodes\n", ++move_num, move_to_string(moves[i]),
             nodes);
      total_nodes += nodes;
    }
    board_unmake_move(board, moves[i]);
  }

  printf("Total nodes at depth %d: %lu\n", depth, total_nodes);
  return;
}

uint64_t safe_perft(Board *board, int depth) {
  if (depth == 0 || board->game_state != GS_ONGOING) {
    return 1ULL;
  }

  uint64_t total_nodes = 0;
  Move moves[MAX_MOVES];
  int num_moves = generate_moves(board, moves);
  if (num_moves == 0) {
    return 1ULL; // checkmate or stalemate
  }

  Board *board_copy = malloc(sizeof(Board));
  memcpy(board_copy, board, sizeof(Board));

  for (int i = 0; i < num_moves; i++) {
    board_make_move(board, moves[i]);
    if (!king_in_check(board, OPPOSITE_COLOR(board->to_move))) {
      total_nodes += safe_perft(board, depth - 1);
    }

    if (!board_valid(board)) {
      fprintf(stderr, "Invalid board after making move %s\n",
              move_to_string(moves[i]));
      printf("Pre:\n%s\n", board_to_string(board_copy));
      printf("Post:\n%s\n", board_to_string(board));
      free(board_copy);
      exit(1);
    }

    board_unmake_move(board, moves[i]);

    if (!board_valid(board)) {
      fprintf(stderr, "Invalid board after unmaking move %s\n",
              move_to_string(moves[i]));
      printf("Pre:\n%s\n", board_to_string(board_copy));
      printf("Post:\n%s\n", board_to_string(board));
      free(board_copy);
      exit(1);
    }

    if (!compare_boards(board, board_copy)) {
      fprintf(stderr, "Board comparison failed after move %s: %s\n",
              move_to_string(moves[i]), last_board_compare_diff());
      free(board_copy);
      exit(1);
    }
  }
  free(board_copy);

  return total_nodes;
}
