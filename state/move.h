//
// Created by Major Zangari on 4/20/25.
//
#ifndef MOVE_H
#define MOVE_H

#include "board.h"
#include <stdint.h>

#define MAX_MOVES 256

typedef uint16_t Move;

int generate_moves(Board *board, Move moves[MAX_MOVES]);

#endif // MOVE_H
