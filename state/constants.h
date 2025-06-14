#ifndef MAUTLIER_CONSTANTS_H
#define MAUTLIER_CONSTANTS_H

#include "board.h"
#include <stdio.h>

extern Bitboard knight_moves[64];
extern Bitboard king_moves[64];

extern const uint64_t rook_magic[64];
extern uint8_t rook_shifts[64];
extern Bitboard rook_blocker_masks[64];
extern Bitboard *rook_attacks[64];

extern const uint64_t bishop_magic[64];
extern uint8_t bishop_shifts[64];
extern Bitboard bishop_blocker_masks[64];
extern Bitboard *bishop_attacks[64];

// MUST CALL BEFORE USING ANY OF THE MOVES
void init_data();

static inline Bitboard get_rook_attack_board(int rook_square, Bitboard occupied,
                                             Bitboard same_side_occupied) {
  Bitboard blocker_mask = rook_blocker_masks[rook_square];
  Bitboard blockers = occupied & blocker_mask;
  uint64_t magic = rook_magic[rook_square];
  int index = (blockers * magic) >> rook_shifts[rook_square];
  Bitboard out = rook_attacks[rook_square][index] & ~same_side_occupied;
  printf(R"(rook square %d, blocker_mask %lu, blockers %lu, index %d, out %lx
)",
         rook_square, blocker_mask, blockers, index, out);
  return out;
}

static inline Bitboard get_bishop_attack_board(int bishop_square,
                                               Bitboard occupied,
                                               Bitboard same_side_occupied) {
  Bitboard blocker_mask = bishop_blocker_masks[bishop_square];
  Bitboard blockers = occupied & blocker_mask;
  uint64_t magic = bishop_magic[bishop_square];
  int index = (blockers * magic) >> bishop_shifts[bishop_square];
  Bitboard out = bishop_attacks[bishop_square][index] & ~same_side_occupied;
  printf(R"(bishop square %d, blockers %lu, index %d, out %lx
)",
         bishop_square, blockers, index, out);
  return out;
}

#endif // MAUTLIER_CONSTANTS_H
