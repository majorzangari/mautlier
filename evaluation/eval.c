#include "eval.h"
#include "bithelpers.h"
#include "board.h"

#define LAZY_EVAL_PAWN 1
#define LAZY_EVAL_KNIGHT 3
#define LAZY_EVAL_BISHOP 3
#define LAZY_EVAL_ROOK 5
#define LAZY_EVAL_QUEEN 9
#define LAZY_EVAL_KING 0

int lazy_evaluation(Board *board) {
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
