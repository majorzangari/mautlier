//
// Created by Major Zangari on 4/19/25.
//

#include "board.h"

Board *fen_to_board(char *fen);
char *board_to_fen(Board *board);
uint64_t square_to_bit(const char *square);
