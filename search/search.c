#include "search.h"
#include "debug_printer.h"
#include "eval.h"
#include "fen.h"
#include "misc.h"
#include "move.h"
#include <stdlib.h>

#define COLOR_MULTIPLIER(color) ((color) == WHITE ? 1 : -1)

// inf might be a misnomer, just a big number to trump other factors
#define INF_SCORE 0xFFFFFF
#define NEG_INF_SCORE -0xFFFFFF

static inline int lazy_search_negamax(Board *board, int depth, int alpha,
                                      int beta) {
  DP_PRINTF("FUNC_TRACE", "lazy_search_negamax\n");
  ToMove color = board->to_move;

  if (depth <= 0) {
    return COLOR_MULTIPLIER(color) * lazy_evaluation(board);
  }

  Move moves[MAX_MOVES];
  int num_moves = generate_moves(board, moves);
  int result = NEG_INF_SCORE;
  for (int i = 0; i < num_moves; i++) {
    char *prefen = board_to_fen(board);

    board_make_move(board, moves[i]);

    if (!board_valid(board)) {
      printf("Invalid board after making move %s\n", move_to_string(moves[i]));
      printf("Board before move: %s\n", prefen);
      exit(1); // TODO: remove
    }

    if (king_in_check(board, color)) {
      board_unmake_move(board, moves[i]);
      continue;
    }

    int new_value = -lazy_search_negamax(board, depth - 1, -beta, -alpha);

    board_unmake_move(board, moves[i]);

    result = MAX(result, new_value);

    alpha = MAX(alpha, result);
    if (alpha >= beta) {
      break;
    }
  }
  return result;
}

Move lazy_search(Board *board, int depth) {
  DP_PRINTF("FUNC_TRACE", "lazy_search\n");

  Move moves[MAX_MOVES];
  int num_moves = generate_moves(board, moves);
  int best_score = NEG_INF_SCORE;
  Move best_move = NULL_MOVE;
  ToMove color = board->to_move;

  for (int i = 0; i < num_moves; i++) {

    board_make_move(board, moves[i]);
    if (king_in_check(board, color)) {
      board_unmake_move(board, moves[i]);
      continue;
    }
    int score =
        -lazy_search_negamax(board, depth - 1, NEG_INF_SCORE, INF_SCORE);
    board_unmake_move(board, moves[i]);

    if (score > best_score) {
      best_score = score;
      best_move = moves[i];
      if (best_score >= INF_SCORE) { // move straight wins, can stop searching,
                                     // TODO: maybe remove?
        return moves[i];
      }
    }
  }
  if (best_move == NULL_MOVE) { // either checkmate or stalemate TODO:handle
    printf("lets fucking go checkmate thing\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    return NULL_MOVE;
  }
  return best_move;
}
