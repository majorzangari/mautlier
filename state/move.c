//
// Created by Major Zangari on 4/20/25.
//

#include "move.h"
#include "board.h"

#define lsb_index(bb) (__builtin_ctzll(bb))
#define pop_lsb(bb) ((bb) &= (bb) - 1)

const Bitboard knight_moves[64] = {0}; // TODO fill in

int generate_pawn_pushes(Bitboard pawns, Bitboard occupied, Move *moves,
                         int color) {
  int num_moves = 0;

  Bitboard empty = ~occupied;

  Bitboard starting_rank = (color == WHITE) ? RANK_2 : RANK_7;

  Bitboard single_push_dest =
      (color == WHITE) ? (pawns << 8) & empty : (pawns >> 8) & empty;

  Bitboard temp_single_dest = single_push_dest;
  while (temp_single_dest) {
    int to_square = lsb_index(temp_single_dest);
    int from_square = (color == WHITE) ? to_square + 8 : to_square - 8;
    *moves++ = (to_square << 10) |
               (from_square << 4); // TODO: add other flags, promotion
    num_moves++;
    pop_lsb(temp_single_dest);
  }

  Bitboard middle_rank = (color == WHITE) ? RANK_3 : RANK_6;
  Bitboard double_push_dest =
      (color == WHITE) ? ((single_push_dest << 8) & empty & middle_rank)
                       : ((single_push_dest >> 8) & empty & middle_rank);

  while (double_push_dest) {
    int to_square = lsb_index(double_push_dest);
    int from_square = (color == WHITE) ? to_square + 16 : to_square - 16;
    *moves++ =
        (to_square << 10) |
        (from_square << 4); // TODO add other flags (might not be necessary)
    num_moves++;
    pop_lsb(double_push_dest);
  }

  return num_moves;
}

int generate_knight_moves(Bitboard knights, Bitboard same_side_occupied,
                          Move *moves) {
  int num_moves = 0;

  while (knights) {
    int index = lsb_index(knights);
    pop_lsb(knights);

    Bitboard attacks = knight_moves[index];
    Bitboard pseudo_moves = attacks & ~same_side_occupied;

    while (pseudo_moves) {
      int to_square = pop_lsb(pseudo_moves);
      pop_lsb(pseudo_moves);
      *moves++ = (index << 10) | (to_square << 4); // TODO: add other flags
      num_moves++;
    }
  }

  return num_moves;
}

int generate_moves(Board *board, Move moves[MAX_MOVES]) {
  int move_count = 0;

  Bitboard knights = board->pieces[board->to_move][KNIGHT];
  Bitboard same_side_occupied = board->occupied_by_color[board->to_move];
  move_count += generate_knight_moves(knights, same_side_occupied, moves);

  return 0; // TODO
}
