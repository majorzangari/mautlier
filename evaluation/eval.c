#include "eval.h"
#include "bithelpers.h"
#include "board.h"
#include "misc.h"
#include "search.h"
#include <math.h>

/* Bulk of eval from Pesto, see:
https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
 */

int mg_value[6] = {82, 337, 365, 477, 1025, 0};
int eg_value[6] = {94, 281, 297, 512, 936, 0};

/* piece/sq tables */
/* values from Rofchade:
 * http://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311&start=19 */
// clang-format off
int mg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0,
};

int eg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
};

int mg_knight_table[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

int eg_knight_table[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

int mg_bishop_table[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

int eg_bishop_table[64] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
};

int mg_rook_table[64] = {
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
};

int eg_rook_table[64] = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};

int mg_queen_table[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

int eg_queen_table[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

int* mg_pesto_table[6] = {
    mg_pawn_table,
    mg_knight_table,
    mg_bishop_table,
    mg_rook_table,
    mg_queen_table,
    mg_king_table
};

int* eg_pesto_table[6] = {
    eg_pawn_table,
    eg_knight_table,
    eg_bishop_table,
    eg_rook_table,
    eg_queen_table,
    eg_king_table
};

int gamephaseInc[6] = {0, 1, 1, 2, 4, 0};
int mg_table[2][6][64];
int eg_table[2][6][64];

// clang-format on
#define FLIP(sq) (sq ^ 56)

// converts my weird index into one that works with the tables above
int convert_to_traditional_index(int index) {
  int rank = index / 8;
  int file = index % 8;
  return (7 - rank) * 8 + (7 - file);
}

void init_eval() {
  for (Piece piece = PAWN; piece <= KING; piece++) {
    for (int sq = 0; sq < 64; sq++) {
      int pesto_index = convert_to_traditional_index(sq);

      // White pieces use Pesto table directly
      mg_table[WHITE][piece][sq] =
          mg_value[piece] + mg_pesto_table[piece][pesto_index];
      eg_table[WHITE][piece][sq] =
          eg_value[piece] + eg_pesto_table[piece][pesto_index];

      // Black pieces mirror the table vertically using ^56
      mg_table[BLACK][piece][sq] =
          mg_value[piece] + mg_pesto_table[piece][pesto_index ^ 56];
      eg_table[BLACK][piece][sq] =
          eg_value[piece] + eg_pesto_table[piece][pesto_index ^ 56];
    }
  }
}

typedef struct {
  int mg;
  int eg;
} Eval;

#define ADD_EVAL(eval1, eval2)                                                 \
  do {                                                                         \
    (eval1).mg += (eval2).mg;                                                  \
    (eval1).eg += (eval2).eg;                                                  \
  } while (0)

#define SUBTRACT_EVAL(eval1, eval2)                                            \
  do {                                                                         \
    (eval1).mg -= (eval2).mg;                                                  \
    (eval1).eg -= (eval2).eg;                                                  \
  } while (0)

// TODO:tune
#define ISOLATED_PAWN_PENALTY_MG 20
#define ISOLATED_PAWN_PENALTY_EG 10
#define DOUBLED_PAWN_PENALTY_MG 15
#define DOUBLED_PAWN_PENALTY_EG 8
#define PASSED_PAWN_BONUS_MG 15
#define PASSED_PAWN_BONUS_EG 25
#define BACKWARD_PAWN_PENALTY_MG 18
#define BACKWARD_PAWN_PENALTY_EG 10

#define CONNECTED_PAWN_BONUS 10

static inline Bitboard adjacent_files(int file) {
  Bitboard mask = 0;
  if (file > 0) {
    mask |= (FILE_H << (file - 1));
  }
  if (file < 7) {
    mask |= (FILE_H << (file + 1));
  }
  return mask;
}

static inline Bitboard same_file_ahead(int file, int index, ToMove color) {
  Bitboard mask = (FILE_H << file);
  if (color == WHITE) {
    mask &= ~((1ULL << index) - 1);
  } else {
    mask &= ((1ULL << index) - 1);
  }
  return mask & ~(1ULL << index); // TODO: last operation might not be necessary
}

static Bitboard behind_mask(int sq, ToMove color) {
  if (color == WHITE) {
    return (1ULL << sq) - 1; // all bits before sq
  } else {
    return ~((1ULL << (sq + 1)) - 1); // all bits after sq
  }
}

static inline Bitboard blocking_pawns_mask(int file, int index, ToMove color) {
  Bitboard front = same_file_ahead(file, index, color);
  Bitboard right = (file > 0) ? same_file_ahead(file - 1, index, color) : 0;
  Bitboard left = (file < 7) ? same_file_ahead(file + 1, index, color) : 0;
  return front | right | left;
}

static inline int distance_from_center(int index) { // TODO: dumb?
  int rank = index / 8;
  int file = index % 8;
  float center_rank = 3.5f;
  float center_file = 3.5f;
  return (int)(fabs(rank - center_rank) + fabs(file - center_file));
}

#define FRONT_PAWN_BONUS 20
#define FRONT_SIDE_PAWN_BONUS 10

#define CASTLE_BONUS_MG 20
#define CASTLE_BONUS_EG 6

#define SEMI_OPEN_PENALTY_MG 12
#define SEMI_OPEN_PENALTY_EG 4
#define OPEN_PENALTY_MG 20
#define OPEN_PENALTY_EG 8

#define BISHOP_MG_PAIR_BONUS 30
#define BISHOP_EG_PAIR_BONUS 40

#define SEMI_OPEN_ROOK_MG 12
#define SEMI_OPEN_ROOK_EG 8
#define OPEN_ROOK_MG 24
#define OPEN_ROOK_EG 18

bool is_endgame(Board *board) {
  int queens = count_set_bits(board->pieces[WHITE][QUEEN]) +
               count_set_bits(board->pieces[BLACK][QUEEN]);
  int minor_major = 0;
  for (Piece piece = KNIGHT; piece <= ROOK; piece++) {
    minor_major += count_set_bits(board->pieces[WHITE][piece]);
    minor_major += count_set_bits(board->pieces[BLACK][piece]);
  }
  return (queens == 0) || (queens == 1 && minor_major <= 1);
}

static inline bool is_castled(const Board *board, int color) {
  int king_sq = lsb_index(board->pieces[color][KING]); // assume only 1 king
  int rank = king_sq / 8;
  int file = king_sq % 8;

  if (color == WHITE && rank == 0) {
    return (file == 6 || file == 2);
  }
  if (color == BLACK && rank == 7) {
    return (file == 6 || file == 2);
  }
  return false;
}

static inline bool king_front_pawn(const Board *board, int color) {
  int king_sq = lsb_index(board->pieces[color][KING]); // assume only 1 king
  int front_pawn_sq = king_sq + (color == WHITE ? 8 : -8);
  return board->pieces[color][PAWN] & (1ULL << front_pawn_sq);
}

static inline int king_side_pawn(const Board *board, int color) {
  int king_index = lsb_index(board->pieces[color][KING]);

  int front_pawn = king_index + (color == WHITE ? 8 : -8);
  int front_right_pawn = front_pawn + 1;
  int front_left_pawn = front_pawn - 1;
  int out = 0;
  if (board->pieces[color][PAWN] & (1ULL << front_right_pawn)) {
    out += 1;
  }
  if (board->pieces[color][PAWN] & (1ULL << front_left_pawn)) {
    out += 1;
  }
  return out;
}

static inline void get_mobility(const Board *board, Features *f) {
  for (ToMove color = WHITE; color <= BLACK; color++) {
    for (Piece piece = KNIGHT; piece <= QUEEN; piece++) {
      Bitboard bitboard = board->pieces[color][piece];
      while (bitboard) {
        int sq = lsb_index(bitboard);
        pop_lsb(bitboard);

        Bitboard attacks = generate_attacks(board, piece, sq, color);
        int mobility = count_set_bits(attacks);

        if (color == WHITE) {
          f->mobility[MOBILITY_INDEX(piece)] += mobility;
        } else {
          f->mobility[MOBILITY_INDEX(piece)] -= mobility;
        }
      }
    }
  }
}

Features extract_features(const Board *board) {
  Features f = {0};

  // Pawn structure
  for (ToMove color = WHITE; color <= BLACK; color++) {
    Bitboard pawns = board->pieces[color][PAWN];

    while (pawns) {
      int sq = lsb_index(pawns);
      pop_lsb(pawns);

      int file = sq % 8;

      if (!(board->pieces[color][PAWN] & adjacent_files(file))) {
        // Isolated pawn
        f.isolated_pawns += COLOR_MULTIPLIER(color);
      }

      if (board->pieces[color][PAWN] & same_file_ahead(file, sq, color)) {
        // Doubled pawn
        f.doubled_pawns += COLOR_MULTIPLIER(color);
      }

      if (!(board->pieces[OPPOSITE_COLOR(color)][PAWN] &
            blocking_pawns_mask(file, sq, color))) {
        // Passed pawn
        f.passed_pawns += COLOR_MULTIPLIER(color);
      }
    }
  }

  // Bishop pair
  if (count_set_bits(board->pieces[WHITE][BISHOP]) >= 2) {
    f.bishop_pair += 1;
  }
  if (count_set_bits(board->pieces[BLACK][BISHOP]) >= 2) {
    f.bishop_pair -= 1;
  }

  // Rooks on open/semi-open files
  for (int color = WHITE; color <= BLACK; color++) {
    Bitboard rooks = board->pieces[color][ROOK];
    while (rooks) {
      int rook_index = lsb_index(rooks);

      Bitboard file_mask = FILE_H << (rook_index % 8);

      if (!(file_mask & board->pieces[color][PAWN])) {
        // semi-open
        if (!(file_mask & board->pieces[OPPOSITE_COLOR(color)][PAWN])) {
          // open
          f.rook_open_file += COLOR_MULTIPLIER(color);
        } else {
          f.rook_semi_open_file += COLOR_MULTIPLIER(color);
        }
      }

      pop_lsb(rooks);
    }
  }

  if (is_castled(board, WHITE)) {
    f.castled += 1;
  }
  if (is_castled(board, BLACK)) {
    f.castled -= 1;
  }

  if (king_front_pawn(board, WHITE)) {
    f.king_front_pawns += 1;
  }
  if (king_front_pawn(board, BLACK)) {
    f.king_front_pawns -= 1;
  }

  f.king_front_side_pawns +=
      king_side_pawn(board, WHITE) - king_side_pawn(board, BLACK);

  get_mobility(board, &f);

  // TODO: center control
  f.tempo = (board->to_move == WHITE) ? 1 : -1;

  return f;
}

int fixed_evaluation(const Board *board) {
  if (board->game_state != GS_ONGOING) {
    switch (board->game_state) {
    case GS_WHITE_WON:
      return MATE_SCORE;
    case GS_BLACK_WON:
      return -MATE_SCORE;
    case GS_DRAW:
      return 0;
    case GS_ONGOING:
      fprintf(stderr, "unreachable\n");
      return 0; // shouldn't happen
    }
  }

  int mg[2];
  int eg[2];
  int game_phase = 0;

  mg[WHITE] = 0;
  mg[BLACK] = 0;
  eg[WHITE] = 0;
  eg[BLACK] = 0;

  for (ToMove color = WHITE; color <= BLACK; color++) {
    for (Piece piece = PAWN; piece <= KING; piece++) {
      Bitboard bitboard = board->pieces[color][piece];
      while (bitboard) {
        int sq = lsb_index(bitboard);
        pop_lsb(bitboard);
        mg[color] += mg_table[color][piece][sq];
        eg[color] += eg_table[color][piece][sq];
        game_phase += gamephaseInc[piece];
      }
    }
  }

  Eval score = {0};
  score.mg = mg[WHITE] - mg[BLACK];
  score.eg = eg[WHITE] - eg[BLACK];

  int mgPhase = game_phase;
  if (mgPhase > 24) {
    mgPhase = 24;
  }
  int egPhase = 24 - mgPhase;
  int out = (score.mg * mgPhase + score.eg * egPhase) / 24;
  return out;
}

int evaluation(const Board *board, double weights[NUM_FEATURES]) {
  if (board->game_state != GS_ONGOING) {
    switch (board->game_state) {
    case GS_WHITE_WON:
      return MATE_SCORE;
    case GS_BLACK_WON:
      return -MATE_SCORE;
    case GS_DRAW:
      return 0;
    case GS_ONGOING:
      fprintf(stderr, "unreachable\n");
      return 0; // shouldn't happen
    }
  }

  int out = fixed_evaluation(board);

  Features features = extract_features(board);
  double feature_eval = evaluate_features(&features, weights);
  out += (int)feature_eval;

  return out;
}
