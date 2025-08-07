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

#define FLAGS_NONE 0
#define FLAGS_DOUBLE_PUSH 1
#define FLAGS_SHORT_CASTLE 2
#define FLAGS_LONG_CASTLE 10
#define FLAGS_KNIGHT_PROMOTION 4
#define FLAGS_BISHOP_PROMOTION 5
#define FLAGS_ROOK_PROMOTION 6
#define FLAGS_QUEEN_PROMOTION 7
#define FLAGS_CAPTURE 8
#define FLAGS_EN_PASSANT 9

#define FLAGS_PAWN_PUSH 3

#define FLAGS_KNIGHT_PROMOTION_CAPTURE 12
#define FLAGS_BISHOP_PROMOTION_CAPTURE 13
#define FLAGS_ROOK_PROMOTION_CAPTURE 14
#define FLAGS_QUEEN_PROMOTION_CAPTURE 15

typedef uint16_t Move;

#define NULL_MOVE 11

int generate_moves(Board *board, Move moves[MAX_MOVES]);

bool king_in_check(Board *board, ToMove color);

char *move_to_string(Move move);

#endif // MOVE_H
