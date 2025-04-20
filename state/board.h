//
// Created by Major Zangari on 4/19/25.
//

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define CR_NONE 0
#define CR_WHITE_SHORT 1
#define CR_WHITE_LONG 2
#define CR_BLACK_SHORT 4
#define CR_BLACK_LONG 8

typedef enum {
  WHITE_TO_MOVE,
  BLACK_TO_MOVE,
} ToMove;

typedef struct {
  uint64_t pieces[2][6];
  ToMove to_move;
  uint8_t castling_rights;
  uint64_t en_passant;
  uint8_t halfmove_clock;
} Board;

const char *board_to_string(Board *board);

#endif // BOARD_H
