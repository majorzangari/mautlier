#include <stdlib.h>

#include "bithelpers.h"
#include "board.h"
#include "hash.h"

uint64_t piece_pos_hash[12][64];
uint64_t side_hash;
uint64_t castling_hash[4];
uint64_t en_passant_hash[8];

uint64_t rand64() { return ((uint64_t)rand() << 32) | rand(); }

void init_zobrist() {
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 64; j++) {
      piece_pos_hash[i][j] = rand64();
    }
  }

  side_hash = rand64();

  for (int i = 0; i < 4; i++) {
    castling_hash[i] = rand64();
  }

  for (int i = 0; i < 8; i++) {
    en_passant_hash[i] = rand64();
  }
}

uint64_t zobrist_hash(const Board *board) {
  uint64_t hash = 0;
  for (int side = WHITE; side <= BLACK; side++) {
    for (Piece piece = PAWN; piece <= KING; piece++) {
      Bitboard piece_bb = board->pieces[side][piece];
      while (piece_bb) {
        int square = lsb_index(piece_bb);
        hash ^= piece_pos_hash[piece + side * 6][square];
        pop_lsb(piece_bb);
      }
    }
  }

  if (board->to_move == BLACK) {
    hash ^= side_hash;
  }

  for (int cr_bit_index = 0; cr_bit_index < 4; cr_bit_index++) {
    uint64_t cr_bit = 1ULL << cr_bit_index;
    if (BOARD_CURR_STATE(board).castling_rights & cr_bit) {
      hash ^= castling_hash[cr_bit_index];
    }
  }

  if (BOARD_CURR_STATE(board).en_passant) {
    int ep_square = lsb_index(BOARD_CURR_STATE(board).en_passant);
    hash ^= en_passant_hash[ep_square % 8]; // Only need the file
  }

  return hash;
}

uint64_t toggle_piece(uint64_t hash, Piece piece, int square, ToMove to_move) {
  return hash ^ piece_pos_hash[piece + to_move * 6][square];
}

uint64_t toggle_castling_rights(uint64_t hash, int castling_rights) {
  return hash ^ castling_hash[castling_rights];
}

uint64_t toggle_turn(uint64_t hash) { return hash ^ side_hash; }

uint64_t toggle_en_passant(uint64_t hash, int square) {
  return hash ^ en_passant_hash[square % 8]; // Only need the file
}
