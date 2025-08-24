#ifndef MAUTLIER_EVAL_H
#define MAUTLIER_EVAL_H

#include "board.h"

#define EVAL_PAWN 100
#define EVAL_KNIGHT 320
#define EVAL_BISHOP 330
#define EVAL_ROOK 500
#define EVAL_QUEEN 900
#define EVAL_KING 20000

static const int piece_values[7] = {
    EVAL_PAWN, EVAL_KNIGHT, EVAL_BISHOP, EVAL_ROOK, EVAL_QUEEN, EVAL_KING, 0};

int lazy_evaluation(Board *board);

#endif
