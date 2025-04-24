//
// Created by Major Zangari on 4/19/25.
//

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define WHITE 0
#define BLACK 1

#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5

#define CR_NONE 0
#define CR_WHITE_SHORT 1
#define CR_WHITE_LONG 2
#define CR_BLACK_SHORT 4
#define CR_BLACK_LONG 8

#define RANK_1 0x00000000000000FFULL
#define RANK_2 0x000000000000FF00ULL
#define RANK_3 0x0000000000FF0000ULL
#define RANK_4 0x00000000FF000000ULL
#define RANK_5 0x000000FF00000000ULL
#define RANK_6 0x0000FF0000000000ULL
#define RANK_7 0x00FF000000000000ULL
#define RANK_8 0xFF00000000000000ULL

#define FILE_A 0x8080808080808080ULL
#define FILE_B 0x4040404040404040ULL
#define FILE_C 0x2020202020202020ULL
#define FILE_D 0x1010101010101010ULL
#define FILE_E 0x0808080808080808ULL
#define FILE_F 0x0404040404040404ULL
#define FILE_G 0x0202020202020202ULL
#define FILE_H 0x0101010101010101ULL

typedef enum {
  WHITE_TO_MOVE,
  BLACK_TO_MOVE,
} ToMove;

typedef uint64_t Bitboard;

typedef struct {
  Bitboard pieces[2][6];
  Bitboard occupied_by_color[2];
  Bitboard occupied;
  ToMove to_move;
  uint8_t castling_rights;
  Bitboard en_passant;
  uint8_t halfmove_clock;
} Board;

void board_update_occupied(Board *board);

const char *board_to_string(Board *board);

#endif // BOARD_H
