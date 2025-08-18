#include "eval.h"
#include "bithelpers.h"
#include "board.h"
#include "search.h"

#define LAZY_EVAL_PAWN 1
#define LAZY_EVAL_KNIGHT 3
#define LAZY_EVAL_BISHOP 3
#define LAZY_EVAL_ROOK 5
#define LAZY_EVAL_QUEEN 9
#define LAZY_EVAL_KING 0

int lazy_evaluation(Board *board) {
  if (board->game_state != GS_ONGOING) {
    switch (board->game_state) {
    case GS_WHITE_WON:
      return INF_SCORE;
    case GS_BLACK_WON:
      return NEG_INF_SCORE;
    case GS_DRAW:
      return 0;
    case GS_ONGOING:
      fprintf(stderr, "unreachable\n");
      return 0; // shouldn't happen
    }
  }

  int out = 0;

  ToMove color = board->to_move;

  // clang-format off
  int pawn_diff   = count_set_bits(board->pieces[WHITE][PAWN])   - count_set_bits(board->pieces[BLACK][PAWN]);
  int knight_diff = count_set_bits(board->pieces[WHITE][KNIGHT]) - count_set_bits(board->pieces[BLACK][KNIGHT]);
  int bishop_diff = count_set_bits(board->pieces[WHITE][BISHOP]) - count_set_bits(board->pieces[BLACK][BISHOP]);
  int rook_diff   = count_set_bits(board->pieces[WHITE][ROOK])   - count_set_bits(board->pieces[BLACK][ROOK]);
  int queen_diff  = count_set_bits(board->pieces[WHITE][QUEEN])  - count_set_bits(board->pieces[BLACK][QUEEN]);
  // clang-format on

  out += LAZY_EVAL_PAWN * pawn_diff;
  out += LAZY_EVAL_KNIGHT * knight_diff;
  out += LAZY_EVAL_BISHOP * bishop_diff;
  out += LAZY_EVAL_ROOK * rook_diff;
  out += LAZY_EVAL_QUEEN * queen_diff;

  // printf("Lazy eval: %d\n", out);
  return out;
}
