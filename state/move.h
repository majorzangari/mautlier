//
// Created by Major Zangari on 4/20/25.
//
#ifndef MAUTLIER_MOVE_H
#define MAUTLIER_MOVE_H

#include "board.h"
#include <stdbool.h>
#include <stdint.h>

// from board.h
typedef struct Board Board;

#define MAX_MOVES 256

#define move_from_square(move) ((move) >> 10)
#define move_to_square(move) ((move >> 4 & 0x3f))
#define move_flags(move) ((move) & 0x0f)

#define FLAGS_NONE (uint16_t)0
#define FLAGS_DOUBLE_PUSH (uint16_t)1
#define FLAGS_SHORT_CASTLE (uint16_t)2
#define FLAGS_LONG_CASTLE (uint16_t)3
#define FLAGS_KNIGHT_PROMOTION (uint16_t)4
#define FLAGS_BISHOP_PROMOTION (uint16_t)5
#define FLAGS_ROOK_PROMOTION (uint16_t)6
#define FLAGS_QUEEN_PROMOTION (uint16_t)7
#define FLAGS_CAPTURE (uint16_t)8
#define FLAGS_EN_PASSANT (uint16_t)9

#define FLAGS_PAWN_PUSH (uint16_t)10

#define FLAGS_KNIGHT_PROMOTION_CAPTURE (uint16_t)12
#define FLAGS_BISHOP_PROMOTION_CAPTURE (uint16_t)13
#define FLAGS_ROOK_PROMOTION_CAPTURE (uint16_t)14
#define FLAGS_QUEEN_PROMOTION_CAPTURE (uint16_t)15

typedef uint16_t Move;

#define NULL_MOVE (uint16_t)11

int generate_moves(Board *board, Move moves[MAX_MOVES]);

bool king_in_check(Board *board, ToMove color);

#endif // MOVE_H
