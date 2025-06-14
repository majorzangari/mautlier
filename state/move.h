//
// Created by Major Zangari on 4/20/25.
//
#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>

// from board.h
typedef struct Board Board;

#define MAX_MOVES 256

#define lsb_index(bb) (__builtin_ctzll(bb))
#define pop_lsb(bb) ((bb) &= (bb) - 1)

#define move_from_square(move) ((move) >> 10)
#define move_to_square(move) ((move >> 4 & 0x3f))
#define move_flags(move) ((move) & 0x0f)

#define FLAGS_NONE 0
#define FLAGS_DOUBLE_PUSH 1
#define FLAGS_SHORT_CASTLE 2
#define FLAGS_LONG_CASTLE 3
#define FLAGS_PROMOTION 4
#define FLAGS_KNIGHT_PROMOTION 4
#define FLAGS_BISHOP_PROMOTION 5
#define FLAGS_ROOK_PROMOTION 6
#define FLAGS_QUEEN_PROMOTION 7
#define FLAGS_EN_PASSANT 9

// only flag that should be mixed with other flags
// all other flags should be used alone
#define FLAGS_CAPTURE 8

typedef uint16_t Move;
int generate_moves(Board *board, Move moves[MAX_MOVES]);

#endif // MOVE_H
