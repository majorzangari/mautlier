#include "search.h"
#include "board.h"
#include "debug_printer.h"
#include "diagnostic_tools.h"
#include "eval.h"
#include "fen.h"
#include "hash.h"
#include "misc.h"
#include "move.h"
#include "transposition_table.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

typedef struct {
  int max_depth; // max depth to search, or 0 for no max depth
  long end_time_ms;
  long nodes_remaining;
  int infinite;
  int stopped;
} SearchInfo;

typedef struct {
  int score;
  Move best_move;
} SearchResults; // TODO: add pv? does that matter?

static inline long get_time_ms() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec * 1000) + (t.tv_usec / 1000);
}

static inline SearchResults search(Board *pos, int depth, int ply, int alpha,
                                   int beta, SearchInfo *info) {

  ToMove color = pos->to_move;
  SearchResults results = {.score = NEG_INF_SCORE, .best_move = NULL_MOVE};
  if (info->stopped)
    return results;
  int has_legal = 0;
  int orig_alpha = alpha;

  uint64_t hash = ZOBRIST_HASH(pos);

  info->nodes_remaining--;
  if ((info->nodes_remaining & 4095) == 0) {
    if (!info->infinite && info->end_time_ms &&
        get_time_ms() > info->end_time_ms) {
      info->stopped = 1;
      return results;
    }
  }

  TTEntry *tt_entry = tt_query(hash);

  // Check TT
  if (tt_entry != NULL && tt_entry->depth >= depth) {
    if (tt_entry->type == TT_EXACT) {
      results.score = get_adjusted_score(tt_entry, ply);
      results.best_move = tt_entry->best_move;
      return results;
    } else if (tt_entry->type == TT_LOWERBOUND) {
      alpha = MAX(alpha, tt_entry->score);
    } else {
      beta = MIN(beta, tt_entry->score);
    }

    if (alpha >= beta) {
      results.score = get_adjusted_score(tt_entry, ply);
      results.best_move = tt_entry->best_move;
      return results;
    }
  }

  // Check depth
  if (depth == 0) {
    results.score = COLOR_MULTIPLIER(color) * lazy_evaluation(pos);
    results.best_move = NULL_MOVE;
    return results;
  }

  Move moves[MAX_MOVES];
  int num_moves = generate_moves(pos, moves);

  // put tt move first
  if (tt_entry != NULL && tt_entry->best_move != NULL_MOVE) {
    for (int i = 0; i < num_moves; i++) {
      if (moves[i] == tt_entry->best_move) {
        moves[i] = moves[0];
        moves[0] = tt_entry->best_move;
        break;
      }
    }
  }

  // recursion loop
  for (int i = 0; i < num_moves; i++) {
    board_make_move(pos, moves[i]);

    if (king_in_check(pos, color)) {
      board_unmake_move(pos, moves[i]);
      continue;
    }

    has_legal = 1;

    SearchResults child_results =
        search(pos, depth - 1, ply + 1, -beta, -alpha, info);
    int score = -child_results.score;

    board_unmake_move(pos, moves[i]);

    if (score > results.score) {
      results.score = score;
      results.best_move = moves[i];
    }

    if (results.score > alpha) {
      alpha = results.score;
    }

    if (alpha >= beta) {
      break; // beta cutoff
    }
  }

  if (!has_legal) {
    if (king_in_check(pos, color)) {
      results.score = NEG_INF_SCORE + ply; // checkmate
    } else {
      results.score = 0;
    }
  }

  tt_store(hash, depth, results.score, orig_alpha, beta, ply,
           results.best_move);

  return results;
}

void search_position(Board *board, SearchRequestInfo *info) {
  long end_time_ms = get_time_ms() + info->max_duration_ms;
  SearchInfo search_info = {
      .max_depth = info->max_depth,
      .end_time_ms = end_time_ms,
      .nodes_remaining = info->max_nodes,
      .infinite = info->infinite,
      .stopped = 0,
  };

  long start_time_ms = get_time_ms();
  Move best_move = NULL_MOVE;
  for (int depth = 1; depth <= MAX_PLY; depth++) {
    SearchResults results =
        search(board, depth, 0, NEG_INF_SCORE, INF_SCORE, &search_info);

    if (!search_info.stopped) {
      best_move = results.best_move;
      printf("info score cp %d depth time %d, %ld pv %s\n", results.score,
             depth, get_time_ms() - start_time_ms,
             move_to_algebraic(results.best_move, board->to_move));
      fflush(stdout);
    }
  }

  printf("bestmove %s\n", move_to_algebraic(best_move, board->to_move));
  fflush(stdout);
}
