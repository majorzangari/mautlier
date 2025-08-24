#include "search.h"
#include "board.h"
#include "debug_printer.h"
#include "diagnostic_tools.h"
#include "eval.h"
#include "fen.h"
#include "misc.h"
#include "move.h"
#include "transposition_table.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

static inline long get_time_ms() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec * 1000) + (t.tv_usec / 1000);
}

#define MAX_PLY 64 // doubt im ever even hitting this

typedef struct {
  int score;
  Move pv[MAX_PLY];
  int pv_length;
} SearchResults;

#define QUEEN_PROMO_BONUS 90000
#define ROOK_PROMO_BONUS 50000
#define KNIGHT_PROMO_BONUS 33000
#define BISHOP_PROMO_BONUS 32000

#define CAPTURE_BONUS 10000

static inline int score_move(Board *pos, Move move) {
  int flags = move_flags(move);

  if ((flags & PROMOTION_MASK) == FLAGS_QUEEN_PROMOTION)
    return QUEEN_PROMO_BONUS;
  if ((flags & PROMOTION_MASK) == FLAGS_ROOK_PROMOTION)
    return ROOK_PROMO_BONUS;
  if ((flags & PROMOTION_MASK) == FLAGS_KNIGHT_PROMOTION)
    return KNIGHT_PROMO_BONUS;
  if ((flags & PROMOTION_MASK) == FLAGS_BISHOP_PROMOTION)
    return BISHOP_PROMO_BONUS;

  if (flags & FLAGS_CAPTURE) {
    if (flags == FLAGS_LONG_CASTLE)
      return 1;
    if (flags == FLAGS_EN_PASSANT)
      return CAPTURE_BONUS;
    int to_square = move_to_square(move);
    int from_square = move_from_square(move);
    Piece captured = pos->piece_table[to_square];
    Piece piece = pos->piece_table[from_square];
    return CAPTURE_BONUS + piece_values[captured] - piece_values[piece];
  }
  if (flags == FLAGS_SHORT_CASTLE)
    return 1;
  return 0;
}

static inline void order_moves(Board *pos, Move *moves, int num_moves) {
  int scores[MAX_MOVES];
  for (int i = 0; i < num_moves; i++) {
    scores[i] = score_move(pos, moves[i]);
  }

  for (int i = 1; i < num_moves; i++) {
    int key = scores[i];
    Move moves_key = moves[i];

    int j = i - 1;
    while (j >= 0 && scores[j] < key) {
      scores[j + 1] = scores[j];
      moves[j + 1] = moves[j];
      j = j - 1;
    }

    scores[j + 1] = key;
    moves[j + 1] = moves_key;
  }
}

static inline SearchResults search(Board *pos, int depth, int ply, int alpha,
                                   int beta, SearchInfo *info) {
  SearchResults results;
  results.score = NEG_INF_SCORE;
  results.pv_length = 0;

  if (info->stopped) {
    return results;
  }

  if (depth <= 0) {
    results.score = COLOR_MULTIPLIER(pos->to_move) * lazy_evaluation(pos);
    return results;
  }

  info->nodes++;
  if ((info->nodes & 4095) == 0) {
    if (!info->infinite && info->endTime && get_time_ms() > info->endTime) {
      info->stopped = 1;
      return results;
    }
    if (info->stop) {
      info->stopped = 1;
      return results;
    }
  }

  ToMove color = pos->to_move;

  uint64_t hash = BOARD_CURR_STATE(pos).hash;
  int alpha_orig = alpha;
  TTEntry *tt_entry = tt_query(hash);
  if (tt_entry && tt_entry->depth >= depth) {
    if (tt_entry->type == TT_EXACT) {
      results.score = tt_entry->score;
      results.pv[0] = tt_entry->best_move; // TODO: figure out rest of pv shit?
      results.pv_length = 1;
      return results;
    }
    if (tt_entry->type == TT_LOWERBOUND) {
      alpha = MAX(alpha, tt_entry->score);
    } else if (tt_entry->type == TT_UPPERBOUND) {
      beta = MIN(beta, tt_entry->score);
    }

    if (alpha >= beta) {
      results.score = tt_entry->score;
      results.pv[0] = tt_entry->best_move;
      results.pv_length = 1;
      return results;
    }
  }

  Move moves[MAX_MOVES];
  int num_moves = generate_moves(pos, moves);
  order_moves(pos, moves, num_moves);
  Move best_move = NULL_MOVE;

  if (tt_entry && tt_entry->best_move != NULL_MOVE) {
    for (int i = 0; i < num_moves; i++) {
      if (moves[i] == tt_entry->best_move) {
        moves[i] = moves[0];
        moves[0] = tt_entry->best_move;
        break;
      }
    }
  }

  for (int i = 0; i < num_moves; i++) {
    Move move = moves[i];
    board_make_move(pos, move);

    if (king_in_check(pos, color)) {
      board_unmake_move(pos, move);
      continue;
    }

    SearchResults child_result =
        search(pos, depth - 1, ply + 1, -beta, -alpha, info);
    board_unmake_move(pos, move);

    int score = -child_result.score;
    if (score > results.score || best_move == NULL_MOVE) {
      results.score = score;
      best_move = move;

      results.pv[0] = move;
      results.pv_length = child_result.pv_length + 1;
      for (int j = 0; j < child_result.pv_length; j++)
        results.pv[j + 1] = child_result.pv[j];
    }

    alpha = MAX(alpha, results.score);
    if (alpha >= beta) {
      break;
    }
  }

  TTEntryType type;
  if (results.score <= alpha_orig)
    type = TT_UPPERBOUND;
  else if (results.score >= beta)
    type = TT_LOWERBOUND;
  else
    type = TT_EXACT;

  tt_store(hash, depth, results.score, type, best_move);

  if (results.score == NEG_INF_SCORE) {
    // no legal moves
    if (king_in_check(pos, color)) {
      results.score = NEG_INF_SCORE + ply; // checkmate
    } else {
      results.score = 0; // stalemate
    }
  }
  return results;
}

void search_position(Board *board, SearchInfo *info) {
  Move best_move = NULL_MOVE;

  info->startTime = get_time_ms();
  info->nodes = 0;
  info->stopped = 0;

  if (info->depth <= 0) {
    info->depth = MAX_PLY; // effectively infinite
  }

  for (int depth = 1; depth <= info->depth; depth++) {
    SearchResults results =
        search(board, depth, 0, NEG_INF_SCORE, INF_SCORE, info);

    if (info->stopped) {
      break;
    }

    best_move = results.pv[0];
    int best_score = results.score;

    long now = get_time_ms();
    printf("info depth %d score cp %d time %ld nodes %ld pv", depth, best_score,
           now - info->startTime, info->nodes);

    for (int i = 0; i < results.pv_length; i++) {
      printf(" %s", move_to_algebraic(results.pv[i], board->to_move));
    }

    printf("\n");
    fflush(stdout);
  }

  printf("bestmove  %s\n", move_to_algebraic(best_move, board->to_move));
  fflush(stdout);
}
