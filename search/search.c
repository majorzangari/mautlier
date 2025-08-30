#include "search.h"
#include "board.h"
#include "eval.h"
#include "hash.h"
#include "misc.h"
#include "move.h"
#include "move_ordering.h"
#include "polyglot.h"
#include "transposition_table.h"

#include <assert.h>
#include <stdio.h>
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
  SearchResults results = {.score = -INF_SCORE, .best_move = NULL_MOVE};
  if (info->stopped)
    return results;
  int has_legal = 0;
  int orig_alpha = alpha;
  int orig_beta = beta;

  uint64_t hash = ZOBRIST_HASH(pos);

  if ((++info->nodes_remaining & 4095) == 0) {
    if (!info->infinite && info->end_time_ms &&
        get_time_ms() > info->end_time_ms) {
      info->stopped = 1;
      return results;
    }
  }

  TTEntry *tt_entry = tt_query(hash);

  // Check depth
  if (depth == 0 || pos->game_state != GS_ONGOING) {
    results.score = COLOR_MULTIPLIER(color) * lazy_evaluation(pos);
    results.best_move = NULL_MOVE;
    return results;
  }

  Move moves[MAX_MOVES];
  int num_moves = generate_moves(pos, moves);

  // Check TT
  if (tt_entry != NULL && tt_entry->depth >= depth) {
    bool found_move = false;
    for (int i = 0; i < num_moves; i++) {
      if (moves[i] == tt_entry->best_move) {
        found_move = true;
        break;
      }
    }

    if (found_move) {
      int tt_score = get_adjusted_score(tt_entry, ply);
      if (tt_entry->type == TT_EXACT) {
        results.score = tt_score;
        results.best_move = tt_entry->best_move;
        return results;
      } else if (tt_entry->type == TT_LOWERBOUND) {
        alpha = MAX(alpha, tt_score);
      } else {
        beta = MIN(beta, tt_score);
      }

      if (alpha >= beta) {
        results.score = get_adjusted_score(tt_entry, ply);
        results.best_move = tt_entry->best_move;
        return results;
      }
    }
  }

  Move tt_move = (tt_entry != NULL) ? tt_entry->best_move : NULL_MOVE;

  int move_scores[MAX_MOVES];
  score_moves(pos, moves, move_scores, num_moves, tt_move, ply);

  // recursion loop
  for (int i = 0; i < num_moves; i++) {
    int move_index =
        pick_next_move(moves, move_scores, num_moves, i, num_moves);
    assert(move_index >= 0 && move_index < num_moves); // TODO: remove
    Move move = moves[move_index];
    board_make_move(pos, move);

    if (king_in_check(pos, color)) {
      board_unmake_move(pos, move);
      continue;
    }

    has_legal = 1;

    SearchResults child_results =
        search(pos, depth - 1, ply + 1, -beta, -alpha, info);
    int score = -child_results.score;

    board_unmake_move(pos, move);

    if (score > results.score) {
      results.score = score;
      results.best_move = move;
    }

    if (results.score > alpha) {
      alpha = results.score;
    }

    if (alpha >= beta) {
      if (!is_capture(move) && !is_promotion(move)) {
        add_killer(move, ply);
        add_history(color, move, depth);
      }
      break; // beta cutoff
    }
  }

  if (!has_legal) {
    if (king_in_check(pos, color)) {
      results.score = -MATE_SCORE + ply; // checkmate
    } else {
      results.score = 0;
    }
  } else {
    tt_store(hash, depth, results.score, orig_alpha, orig_beta, ply,
             results.best_move);
  }

  return results;
}

void search_position(Board *board, SearchRequestInfo *info,
                     FILE *opening_book) {
  long end_time_ms = get_time_ms() + info->max_duration_ms;
  SearchInfo search_info = {
      .max_depth = info->max_depth,
      .end_time_ms = end_time_ms,
      .nodes_remaining = 0, // TODO: change
      .infinite = info->infinite,
      .stopped = 0,
  };

  uint64_t hash = ZOBRIST_HASH(board);

  if (opening_book != NULL && board->full_move_clock < 10) {
    printf("info string looking for book move\n");
    fflush(stdout);
    Move move = lookup_book_move(hash, opening_book, board);
    if (move != NULL_MOVE) {
      printf("info score cp 0 depth 0 time 0 nodes 0 pv %s\n",
             move_to_algebraic(move, board->to_move));
      printf("bestmove %s\n", move_to_algebraic(move, board->to_move));
      fflush(stdout);
      return;
    } else {
      printf("info string no book move found\n");
      fflush(stdout);
    }
  }

  long start_time_ms = get_time_ms();
  Move best_move = NULL_MOVE;
  for (int depth = 1; depth <= MAX_PLY; depth++) {
    if (search_info.stopped)
      break;
    SearchResults results =
        search(board, depth, 0, -INF_SCORE, INF_SCORE, &search_info);

    if (!search_info.stopped || best_move == NULL_MOVE) {
      best_move = results.best_move;
      printf("info score cp %d depth %d time %ld nodes %ld pv %s\n",
             results.score, depth, get_time_ms() - start_time_ms,
             search_info.nodes_remaining,
             move_to_algebraic(results.best_move, board->to_move));
      fflush(stdout);
    }
  }

  printf("bestmove %s\n", move_to_algebraic(best_move, board->to_move));
  fflush(stdout);
}
