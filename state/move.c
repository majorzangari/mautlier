//
// Created by Major Zangari on 4/20/25.
//

#include "move.h"
#include "board.h"
#include "constants.h"

static inline Move encode_move(int from_square, int to_square, int flags) {
  return (from_square << 10) | (to_square << 4) | flags;
}

int generate_pawn_pushes(Bitboard pawns, Bitboard occupied, Move *moves,
                         ToMove color) {
  int num_moves = 0;

  Bitboard empty = ~occupied;

  Bitboard starting_rank = (color == WHITE) ? RANK_2 : RANK_7;

  Bitboard single_push_dest =
      (color == WHITE) ? (pawns << 8) & empty : (pawns >> 8) & empty;

  Bitboard temp_single_dest = single_push_dest;
  while (temp_single_dest) {
    int to_square = lsb_index(temp_single_dest);
    int from_square = (color == WHITE) ? to_square + 8 : to_square - 8;
    *moves++ = encode_move(from_square, to_square,
                           0); // TODO: add other flags, promotion
    num_moves++;
    pop_lsb(temp_single_dest);
  }

  Bitboard double_rank = (color == WHITE) ? RANK_4 : RANK_5;
  Bitboard double_push_dest =
      (color == WHITE) ? ((single_push_dest << 8) & empty & double_rank)
                       : ((single_push_dest >> 8) & empty & double_rank);
  while (double_push_dest) {
    int to_square = lsb_index(double_push_dest);
    int from_square = (color == WHITE) ? to_square + 16 : to_square - 16;
    *moves++ = encode_move(from_square, to_square,
                           0); // TODO: add other flags (might not be necessary)
    num_moves++;
    pop_lsb(double_push_dest);
  }

  return num_moves;
}

int generate_pawn_captures(Bitboard pawns, Bitboard enemy_occupied, Move *moves,
                           ToMove color) {
  int num_moves = 0;

  Bitboard left_capture = (color == WHITE)
                              ? (pawns << 9) & enemy_occupied & ~FILE_H
                              : (pawns >> 7) & enemy_occupied & ~FILE_H;
  Bitboard right_capture = (color == WHITE)
                               ? (pawns << 7) & enemy_occupied & ~FILE_A
                               : (pawns >> 9) & enemy_occupied & ~FILE_A;

  while (left_capture) {
    int to_square = lsb_index(left_capture);
    int from_square = (color == WHITE) ? to_square - 9 : to_square + 7;
    *moves++ = encode_move(from_square, to_square,
                           1); // TODO: add other flags CAPTURE
    num_moves++;
    pop_lsb(left_capture);
  }

  while (right_capture) {
    int to_square = lsb_index(right_capture);
    int from_square = (color == WHITE) ? to_square - 7 : to_square + 9;
    *moves++ = encode_move(from_square, to_square,
                           1); // TODO: add other flags CAPTURE
    num_moves++;
    pop_lsb(right_capture);
  }

  return num_moves;
}

int generate_en_passant(Bitboard pawns, Bitboard ep, ToMove color,
                        Move *moves) {
  if (!ep)
    return 0;

  int num_moves = 0;
  int ep_square = lsb_index(ep);

  Bitboard attackers;
  if (color == WHITE) {
    Bitboard r5_pawns = pawns & RANK_5;
    Bitboard right_attackers = (ep >> 7) & r5_pawns & ~FILE_A;
    Bitboard left_attackers = (ep >> 9) & r5_pawns & ~FILE_H;
    attackers = left_attackers | right_attackers;
  } else {
    Bitboard r4_pawns = pawns & RANK_4;
    Bitboard right_attackers = (ep << 9) & r4_pawns & ~FILE_H;
    Bitboard left_attackers = (ep << 7) & r4_pawns & ~FILE_A;
    attackers = left_attackers | right_attackers;
  }

  while (attackers) {
    int from_sq = lsb_index(attackers);
    *moves++ = encode_move(from_sq, ep_square,
                           0); // TODO: add other flags CAPTURE
    num_moves++;
    pop_lsb(attackers);
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
      int to_square = lsb_index(pseudo_moves);
      pop_lsb(pseudo_moves);
      *moves++ =
          encode_move(index, to_square,
                      0); // TODO: add other flags (might not be necessary)
      num_moves++;
    }
  }

  return num_moves;
}

int generate_king_moves(Bitboard kings, Bitboard same_side_occupied,
                        Move *moves) {
  int num_moves = 0;
  while (kings) {
    int index = lsb_index(kings);
    pop_lsb(kings);

    Bitboard attacks = king_moves[index];
    Bitboard pseudo_moves = attacks & ~same_side_occupied;

    while (pseudo_moves) {
      int to_square = lsb_index(pseudo_moves);
      pop_lsb(pseudo_moves);
      *moves++ = encode_move(index, to_square, 0); // TODO: add other flags
      num_moves++;
    }
  }

  return num_moves;
}

int generate_rook_moves(Bitboard rooks, Bitboard occupied,
                        Bitboard same_side_occupied, Move *moves) {
  int num_moves = 0;

  while (rooks) {
    int index = lsb_index(rooks);
    pop_lsb(rooks);
    Bitboard rook_moves =
        get_rook_attack_board(index, occupied, same_side_occupied);

    while (rook_moves) {
      int to_square = lsb_index(rook_moves);
      pop_lsb(rook_moves);
      *moves++ = encode_move(index, to_square, 0); // TODO: add other flags
      num_moves++;
    }
  }

  return num_moves;
}

int generate_moves(Board *board, Move moves[MAX_MOVES]) {
  int move_count = 0;

  Bitboard pawns = board->pieces[board->to_move][PAWN];
  Bitboard knights = board->pieces[board->to_move][KNIGHT];
  Bitboard rooks = board->pieces[board->to_move][ROOK];
  Bitboard queens = board->pieces[board->to_move][QUEEN];
  Bitboard kings = board->pieces[board->to_move][KING];
  Bitboard same_side_occupied = board->occupied_by_color[board->to_move];
  Bitboard enemy_side_occupied = board->occupied_by_color[1 - board->to_move];

  move_count += generate_knight_moves(knights, same_side_occupied, moves);
  move_count += generate_pawn_pushes(pawns, board->occupied, moves + move_count,
                                     board->to_move);
  move_count += generate_pawn_captures(pawns, enemy_side_occupied,
                                       moves + move_count, board->to_move);
  move_count += generate_en_passant(pawns, board->en_passant, board->to_move,
                                    moves + move_count);
  move_count +=
      generate_king_moves(kings, same_side_occupied, moves + move_count);
  move_count += generate_rook_moves(rooks | queens, board->occupied,
                                    same_side_occupied, moves + move_count);

  *(moves + move_count) = (uint16_t)0; // TEMP
  return move_count;
}
