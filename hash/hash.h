#include <stdint.h>

#include "board.h"

#define ZOBRIST_WHITE_PAWN 0
#define ZOBRIST_WHITE_KNIGHT 1
#define ZOBRIST_WHITE_BISHOP 2
#define ZOBRIST_WHITE_ROOK 3
#define ZOBRIST_WHITE_QUEEN 4
#define ZOBRIST_WHITE_KING 5
#define ZOBRIST_BLACK_PAWN 6
#define ZOBRIST_BLACK_KNIGHT 7
#define ZOBRIST_BLACK_BISHOP 8
#define ZOBRIST_BLACK_ROOK 9
#define ZOBRIST_BLACK_QUEEN 10
#define ZOBRIST_BLACK_KING 11

#define ZOBRIST_H_FILE 0
#define ZOBRIST_G_FILE 1
#define ZOBRIST_F_FILE 2
#define ZOBRIST_E_FILE 3
#define ZOBRIST_D_FILE 4
#define ZOBRIST_C_FILE 5
#define ZOBRIST_B_FILE 6
#define ZOBRIST_A_FILE 7

#define ZOBRIST_CR_WHITE_SHORT 0
#define ZOBRIST_CR_WHITE_LONG 1
#define ZOBRIST_CR_BLACK_SHORT 2
#define ZOBRIST_CR_BLACK_LONG 3

// MUST BE CALLED BEFORE USING ANY HASH FUNCTIONS
void init_zobrist();

uint64_t zobrist_hash(const Board *board);

// for adding or removing a piece
uint64_t toggle_piece(uint64_t hash, Piece piece, int square, ToMove to_move);

// shortcut for moving a piece in hash
// NOTE: does not account for captures
static inline uint64_t move_piece_hash(uint64_t hash, Piece piece,
                                       int from_square, int to_square,
                                       ToMove to_move) {
  uint64_t hash2 = toggle_piece(hash, piece, from_square, to_move);
  return toggle_piece(hash2, piece, to_square, to_move);
}
// toggle a single castling right in hash
// note: USE ZOBRIST macros
uint64_t toggle_castling_rights(uint64_t hash, int castling_rights);

uint64_t toggle_turn(uint64_t hash);

uint64_t toggle_en_passant(uint64_t hash, int square);
