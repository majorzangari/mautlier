//
// Created by Major Zangari on 4/20/25.
//

#include "board.h"
#include <stdint.h>

#define MAX_MOVES 256

typedef uint16_t Move;

int generate_moves(Board *board, Move moves[MAX_MOVES]);
