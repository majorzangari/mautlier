#include "move_ordering.h"
#include "move.h"
#include "search.h"
#include <stdlib.h>

#define KILLERS_PER_PLY 2
#define HISTORY_MAX 16384

static Move killer[MAX_PLY][KILLERS_PER_PLY];
static int history[2][64][64]; // side, from, to
// static Move counter_move[2][6][64]; // side, piece, to

static const int mvv_lva[6][6] = { // victim, attacker
    /* Victim P */ {105, 205, 305, 405, 505, 605},
    /* Victim N */ {204, 304, 404, 504, 604, 704},
    /* Victim B */ {203, 303, 403, 503, 603, 703},
    /* Victim R */ {402, 502, 602, 702, 802, 902},
    /* Victim Q */ {501, 601, 701, 801, 901, 1001},
    /* Victim K */ {600, 700, 800, 900, 1000, 1100}};

int *score_moves(Board *pos, Move *moves, int scores[MAX_MOVES], int num_moves,
                 Move tt_move, int ply) {

  for (int i = 0; i < num_moves; i++) {
    Move move = moves[i];
    if (move == tt_move) {
      scores[i] = 1 << 20;
      continue;
    }

    int promotion = is_promotion(move);
    if (promotion) {
      int base = 10000;
      if (is_capture(move)) {
        int from = move_from_square(move);
        int to = move_to_square(move);
        Piece victim = pos->piece_table[to];
        Piece attacker = pos->piece_table[from];

        int capture_score = mvv_lva[victim][attacker];
        scores[i] = base + capture_score +
                    (promotion * 100); // TODO: test different values here
      } else {
        scores[i] = base + promotion * 100;
      }
      continue;
    }

    if (is_capture(move)) {
      int from = move_from_square(move);
      int to = move_to_square(move);
      int flags = move_flags(move);
      Piece victim = (flags == FLAGS_EN_PASSANT) ? PAWN : pos->piece_table[to];
      Piece attacker = pos->piece_table[from];
      int capture_score = mvv_lva[victim][attacker];
      scores[i] = capture_score;
      continue;
    }

    bool is_killer = false;
    for (int j = 0; j < KILLERS_PER_PLY; j++) {
      if (killer[ply][j] == move) {
        scores[i] = 9999; // assign to this move's score
        is_killer = true;
        break;
      }
    }

    if (is_killer) {
      continue;
    }

    int from = move_from_square(move);
    int to = move_to_square(move);
    int s = history[pos->to_move][from][to];
    scores[i] = 1000 + s;
  }

  return scores;
}

int pick_next_move(Move *moves, int *scores, int num_moves, int start,
                   int end) {
  int best_index = start;
  int best_score = scores[start];
  for (int i = start + 1; i < end; i++) {
    if (scores[i] > best_score) {
      best_index = i;
      best_score = scores[i];
    }
  }

  if (best_index != start) {
    Move temp_move = moves[start];
    int temp_score = scores[start];

    moves[start] = moves[best_index];
    scores[start] = best_score;

    moves[best_index] = temp_move;
    scores[best_index] = temp_score;
  }

  return start;
}

void add_killer(Move move, int depth) {
  if (killer[depth][0] != move) {
    killer[depth][1] = killer[depth][0];
    killer[depth][0] =
        move; // TODO: adapt for potentially greater killers per ply
  }
}

void add_history(ToMove side, Move move, int depth) {
  int from = move_from_square(move);
  int to = move_to_square(move);
  int *entry = &history[side][from][to];
  *entry += depth * depth;

  if (*entry > HISTORY_MAX) {
    for (int side_i = WHITE; side_i <= BLACK; side_i++) {
      for (int from_i = 0; from_i < 64; from_i++) {
        for (int to_i = 0; to_i < 64; to_i++) {
          history[side_i][from_i][to_i] >>= 1;
        }
      }
    }
  }
}
