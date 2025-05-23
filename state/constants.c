#include "constants.h"
#include "board.h"
#include <stdlib.h>

Bitboard knight_moves[64];
Bitboard king_moves[64];

const uint64_t rook_magic[64] = {
    0x800080c00b2010,   0x4040009004402000, 0x500090041200012,
    0x80080080061000,   0xa000a0008902014,  0x1200081406001011,
    0x400102822040a81,  0x100022602408100,  0xc00a800420400080,
    0x2100802002804010, 0x20802000100081,   0x8801000180280,
    0x182000a00242010,  0x200808024002200,  0x201000100141600,
    0x200200014204128b, 0x8000450021008004, 0x4c04000201001,
    0x20010011042142,   0x2000818008001000, 0x1001808004007800,
    0x4004002004100,    0xa002040085100248, 0x6010020005a94403,
    0x5004800080204004, 0x8120022140005001, 0xc8100080200280,
    0x20120420011ca00,  0x28028080040018,   0x2000200081004,
    0x2800120400100108, 0x4205605600010084, 0x70c00882800020,
    0x3000200180804000, 0x530841001802002,  0x8006003042002048,
    0x4204000480800800, 0x1208402018010410, 0x4105086204005003,
    0x2200800160800100, 0x400184c004208000, 0x800600850004003,
    0x849108200420020,  0x2101a010010019,   0x12001008220004,
    0x3202002010040400, 0x280a010002008080, 0x854000905112000c,
    0x2822062900814200, 0x281002600844200,  0x8000842000100080,
    0x82801001480080,   0xc0008000c008080,  0x1008040080260080,
    0x410c10182102b400, 0x204800100004080,  0x48000150024c1,
    0x850040022011,     0x820102000410009,  0x25008300061010d,
    0x20080010050105,   0x682d000882040011, 0x8803028020a8914,
    0x20010400288042,
};
uint8_t rook_shifts[64];
Bitboard rook_blocker_masks[64];
Bitboard *rook_attacks[64];

const uint64_t bishop_magic[64] = {
    0x4010240488021022, 0x1004040400420400, 0x8010044150408000,
    0x4008094104009000, 0x8001104018800108, 0x886028200820,
    0x4000421004200000, 0x100662010c014001, 0x2400408881800a4,
    0x10090501120200,   0x4002040c00820830, 0x480841014200,
    0x404a00004a8,      0x1020590420060583, 0x484208090105110,
    0x18209042a0200,    0x106040810440814,  0xc1004201a00a102,
    0x402002408001100,  0x802240802004040,  0x2204019882a04c00,
    0x10080111001a000,  0x8004000212210481, 0xd106205420210,
    0x1b344080200a24a0, 0x8a082040a4010202, 0x48280030004141,
    0x8486080104004008, 0x8000940001802000, 0x2000410102100221,
    0x344008000480412,  0x1000802442050400, 0x81141000206000,
    0x808901000241442,  0x408042480c5000e4, 0xa040400888820200,
    0x4010601040048,    0xc0b024202010101,  0x100ba100089400,
    0x800104002001010a, 0x4100808040400,    0x400840108582084,
    0x800620022001015,  0x42010141080800,   0xa000041008800402,
    0x2048081004100020, 0x2150100120408922, 0x1a044400200280,
    0x582880510500010,  0x20202310481c1,    0x4608140000,
    0x1802000120980028, 0x201268a1010020,   0x200400801810000,
    0x402208a1230000,   0x808c800841042c1,  0x91014a00844015,
    0x500480c0440,      0x400208c5008,      0x1601004010840408,
    0x2060040114a0200,  0x200805020220c20,  0x80b400448420040,
    0x408c84081840300,
};
uint8_t bishop_shifts[64];
Bitboard bishop_blocker_masks[64];
Bitboard *bishop_attacks[64];

void init_knight_moves() {
  int knight_pattern[][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1},
                             {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 7; file >= 0; file--) {
      Bitboard moves = 0;

      for (int i = 0; i < 8; i++) {
        int new_rank = rank + knight_pattern[i][0];
        int new_file = file + knight_pattern[i][1];

        if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
          moves |= (1ULL << (new_rank * 8 + new_file));
        }
      }

      int square_index = rank * 8 + file;
      knight_moves[square_index] = moves;
    }
  }
}

void init_king_moves() {
  int king_pattern[][2] = {{1, 0}, {0, 1},  {-1, 0}, {0, -1},
                           {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 7; file >= 0; file--) {
      Bitboard moves = 0;

      for (int i = 0; i < 8; i++) {
        int new_rank = rank + king_pattern[i][0];
        int new_file = file + king_pattern[i][1];

        if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
          moves |= (1ULL << (new_rank * 8 + new_file));
        }
      }

      int square_index = rank * 8 + file;
      king_moves[square_index] = moves;
    }
  }
}

void init_rook_shifts() {
  uint8_t base = 10;
  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 7; file >= 0; file--) {
      uint8_t adjusted = base;
      if (rank == 0 || rank == 7)
        adjusted++;
      if (file == 0 || file == 7)
        adjusted++;
      rook_shifts[rank * 8 + file] = 64 - adjusted;
    }
  }
}

void init_rook_blockers() {
  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 7; file >= 0; file--) {
      Bitboard rank_bb = RANK_1 << (8 * rank);
      Bitboard file_bb = FILE_H << file;
      Bitboard attacking_squares = rank_bb | file_bb;
      Bitboard blockers = attacking_squares;
      if (rank != 7)
        blockers &= ~RANK_8;
      if (rank != 0)
        blockers &= ~RANK_1;
      if (file != 7)
        blockers &= ~FILE_A;
      if (file != 0)
        blockers &= FILE_H;

      int index = rank * 8 + file;
      blockers &= ~(1ULL << index);
      rook_blocker_masks[index] = blockers;
    }
  }
}

Bitboard get_rook_attacks(Bitboard blockers, int square_index) {
  Bitboard attacks = 0ULL;
  int rank = square_index / 8;
  int file = square_index % 8;

  for (int r = rank + 1; r < 8; r++) {
    int sq = r * 8 + file;
    attacks |= (1ULL << sq);
    if (blockers & (1ULL << sq))
      break;
  }

  for (int r = rank - 1; r >= 0; r--) {
    int sq = r * 8 + file;
    attacks |= (1ULL << sq);
    if (blockers & (1ULL << sq))
      break;
  }

  for (int f = file + 1; f < 8; f++) {
    int sq = rank * 8 + f;
    attacks |= (1ULL << sq);
    if (blockers & (1ULL << sq))
      break;
  }

  for (int f = file - 1; f >= 0; f--) {
    int sq = rank * 8 + f;
    attacks |= (1ULL << sq);
    if (blockers & (1ULL << sq))
      break;
  }
  return attacks;
}

void init_rook_attacks(int square_index) {
  uint64_t magic = rook_magic[square_index];
  uint8_t shift = rook_shifts[square_index];
  Bitboard blocker_mask = rook_blocker_masks[square_index];
  rook_attacks[square_index] = malloc(sizeof(Bitboard) * (1 << (64 - shift)));

  Bitboard blockers = 0;
  do {
    int index = (blockers * magic) >> shift;
    rook_attacks[square_index][index] =
        get_rook_attacks(blockers, square_index);
    blockers = (blockers - blocker_mask) & blocker_mask;
  } while (blockers);
}

void init_rook_moves() {
  init_rook_shifts();
  init_rook_blockers();
  for (int square_index = 0; square_index < 64; square_index++) {
    init_rook_attacks(square_index);
  }
}

void init_bishop_shifts() {
  for (int square_index = 0; square_index < 64; square_index++) {
    int index_bits = 5;
    if (square_index == 0 || square_index == 7 || square_index == 56 ||
        square_index == 63) { // corners
      index_bits = 6;
    }
    int rank = square_index / 8;
    int file = square_index % 8;
    if (rank >= 2 && rank <= 5 && file >= 2 && file <= 5) { // center 4x4
      index_bits = 7;
    }
    if (rank >= 3 && rank <= 4 && file >= 3 && file <= 4) { // center 2x2
      index_bits = 9;
    }

    bishop_shifts[square_index] = 64 - index_bits;
  }
}

void init_bishop_blockers() {
  int directions[4][2] = {
      {1, 1},
      {1, -1},
      {-1, 1},
      {-1, -1},
  };

  for (int square_index = 0; square_index < 64; square_index++) {
    Bitboard mask = 0;
    int rank = square_index / 8;
    int file = square_index % 8;
    for (size_t j = 0; j < 4; j++) {
      int dr = directions[j][0];
      int df = directions[j][1];
      for (size_t k = 1; k < 8; k++) {
        int r = rank + (k * dr);
        int f = file + (k * df);
        if (r > 7 || f > 7 || r < 0 || f < 0) {
          break;
        }
        mask |= (1ULL << (r * 8 + f));
      }
    }
    mask &= ~(1ULL << square_index);
    mask &= ~(RANK_1 | RANK_8 | FILE_A | FILE_H);
    bishop_blocker_masks[square_index] = mask;
  }
}

Bitboard get_bishop_attacks(Bitboard blockers, int square_index) {
  Bitboard attacks = 0ULL;

  int rank = square_index / 8;
  int file = square_index % 8;

  int directions[4][2] = {
      {1, 1},
      {1, -1},
      {-1, 1},
      {-1, -1},
  };

  for (size_t i = 0; i < 4; i++) {
    int dr = directions[i][0];
    int df = directions[i][1];
    for (size_t j = 0; j < 8; j++) {
      int r = rank + (j * dr);
      int f = file + (j * df);
      if (r > 7 || f > 7 || r < 0 || f < 0) {
        break;
      }

      int sq = r * 8 + f;
      Bitboard square_bit = (1ULL << sq);
      attacks |= square_bit;
      if ((blockers & square_bit) != 0) {
        break;
      }
    }
  }

  return attacks;
}

void init_bishop_attacks(int square_index) {
  uint64_t magic = bishop_magic[square_index];
  uint8_t shift = bishop_shifts[square_index];
  Bitboard blocker_mask = bishop_blocker_masks[square_index];
  bishop_attacks[square_index] = malloc(sizeof(Bitboard) * (1 << (64 - shift)));

  Bitboard blockers = 0;
  do {
    int index = (blockers * magic) >> shift;
    bishop_attacks[square_index][index] =
        get_bishop_attacks(blockers, square_index);
    blockers = (blockers - blocker_mask) & blocker_mask;
  } while (blockers);
}

void init_bishop_moves() {
  init_bishop_shifts();
  init_bishop_blockers();
  for (int square_index = 0; square_index < 64; square_index++) {
    init_bishop_attacks(square_index);
  }
}

void init_data() {
  init_knight_moves();
  init_king_moves();
  init_rook_moves();
  init_bishop_moves();
}
