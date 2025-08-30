//
// Created by Major Zangari on 4/20/25.
//

#include "move.h"
#include "bithelpers.h"
#include "board.h"
#include "constants.h"
#include "debug_printer.h"
#include "diagnostic_tools.h"
#include "misc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static inline Move encode_move(int from_square, int to_square, int flags) {
  return (from_square << 10) | (to_square << 4) | flags;
}

int generate_pawn_pushes(Bitboard pawns, Bitboard occupied, Move *moves,
                         ToMove color) {
  DP_PRINTF("FUNC_TRACE", "generate_pawn_pushes\n");
  int num_moves = 0;

  Bitboard empty = ~occupied;

  Bitboard starting_rank = (color == WHITE) ? RANK_2 : RANK_7;

  Bitboard single_push_dest =
      (color == WHITE) ? (pawns << 8) & empty : (pawns >> 8) & empty;

  Bitboard temp_single_dest = single_push_dest;
  while (temp_single_dest) {
    int to_square = lsb_index(temp_single_dest);
    int from_square = (color == WHITE) ? to_square - 8 : to_square + 8;
    bool promote = (color == WHITE) ? to_square >= 56 : to_square <= 7;
    if (promote) {
      *moves++ = encode_move(from_square, to_square, FLAGS_QUEEN_PROMOTION);
      *moves++ = encode_move(from_square, to_square, FLAGS_ROOK_PROMOTION);
      *moves++ = encode_move(from_square, to_square, FLAGS_KNIGHT_PROMOTION);
      *moves++ = encode_move(from_square, to_square, FLAGS_BISHOP_PROMOTION);
      num_moves += 4;
    } else {
      *moves++ = encode_move(from_square, to_square, FLAGS_PAWN_PUSH);
      num_moves++;
    }
    pop_lsb(temp_single_dest);
  }

  Bitboard double_rank = (color == WHITE) ? RANK_4 : RANK_5;
  Bitboard double_push_dest =
      (color == WHITE) ? ((single_push_dest << 8) & empty & double_rank)
                       : ((single_push_dest >> 8) & empty & double_rank);
  while (double_push_dest) {
    int to_square = lsb_index(double_push_dest);
    int from_square = (color == WHITE) ? to_square - 16 : to_square + 16;
    *moves++ = encode_move(from_square, to_square, FLAGS_DOUBLE_PUSH);
    num_moves++;
    pop_lsb(double_push_dest);
  }

  return num_moves;
}

int generate_pawn_captures(Bitboard pawns, Bitboard enemy_occupied, Move *moves,
                           ToMove color) {
  DP_PRINTF("FUNC_TRACE", "generate_pawn_captures\n");
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
    bool promote = (color == WHITE) ? to_square >= 56 : to_square <= 7;
    if (promote) {
      // clang-format off
      *moves++ = encode_move(from_square, to_square, FLAGS_QUEEN_PROMOTION_CAPTURE);
      *moves++ = encode_move(from_square, to_square, FLAGS_ROOK_PROMOTION_CAPTURE);
      *moves++ = encode_move(from_square, to_square, FLAGS_KNIGHT_PROMOTION_CAPTURE);
      *moves++ = encode_move(from_square, to_square, FLAGS_BISHOP_PROMOTION_CAPTURE);
      // clang-format on
      num_moves += 4;
    } else {
      *moves++ = encode_move(from_square, to_square, FLAGS_CAPTURE);
      num_moves++;
    }
    pop_lsb(left_capture);
  }

  while (right_capture) {
    int to_square = lsb_index(right_capture);
    int from_square = (color == WHITE) ? to_square - 7 : to_square + 9;
    bool promote = (color == WHITE) ? to_square >= 56 : to_square <= 7;
    if (promote) {
      *moves++ =
          encode_move(from_square, to_square, FLAGS_KNIGHT_PROMOTION_CAPTURE);
      *moves++ =
          encode_move(from_square, to_square, FLAGS_BISHOP_PROMOTION_CAPTURE);
      *moves++ =
          encode_move(from_square, to_square, FLAGS_ROOK_PROMOTION_CAPTURE);
      *moves++ =
          encode_move(from_square, to_square, FLAGS_QUEEN_PROMOTION_CAPTURE);
      num_moves += 4;
    } else {
      *moves++ = encode_move(from_square, to_square, FLAGS_CAPTURE);
      num_moves++;
    }
    pop_lsb(right_capture);
  }

  return num_moves;
}

int generate_en_passant(Bitboard pawns, Bitboard ep, ToMove color,
                        Move *moves) {
  DP_PRINTF("FUNC_TRACE", "generate_en_passant\n");
  if (!ep)
    return 0;

  int num_moves = 0;
  int ep_square = lsb_index(ep);

  Bitboard attackers;
  if (color == WHITE) {
    Bitboard r5_pawns = pawns & RANK_5;
    Bitboard right_attackers = (ep >> 7) & r5_pawns & ~FILE_H;
    Bitboard left_attackers = (ep >> 9) & r5_pawns & ~FILE_A;
    attackers = left_attackers | right_attackers;
  } else {
    Bitboard r4_pawns = pawns & RANK_4;
    Bitboard right_attackers = (ep << 9) & r4_pawns & ~FILE_H;
    Bitboard left_attackers = (ep << 7) & r4_pawns & ~FILE_A;
    attackers = left_attackers | right_attackers;
  }

  while (attackers) {
    int from_sq = lsb_index(attackers);
    *moves++ = encode_move(from_sq, ep_square, FLAGS_EN_PASSANT);
    num_moves++;
    pop_lsb(attackers);
  }

  return num_moves;
}

int generate_knight_moves(Bitboard knights, Bitboard same_side_occupied,
                          Bitboard enemy_occupied, Move *moves) {
  DP_PRINTF("FUNC_TRACE", "generate_knight_moves\n");
  int num_moves = 0;

  while (knights) {
    int index = lsb_index(knights);
    pop_lsb(knights);

    Bitboard attacks = knight_moves[index];
    Bitboard pseudo_moves = attacks & ~same_side_occupied;

    while (pseudo_moves) {
      int to_square = lsb_index(pseudo_moves);
      pop_lsb(pseudo_moves);
      if (enemy_occupied & (1ULL << to_square)) {
        *moves++ = encode_move(index, to_square, FLAGS_CAPTURE);
      } else {
        *moves++ = encode_move(index, to_square, 0);
      }
      num_moves++;
    }
  }

  return num_moves;
}

int generate_king_moves(Bitboard kings, Bitboard same_side_occupied,
                        Bitboard enemy_occupied, Move *moves) {
  DP_PRINTF("FUNC_TRACE", "generate_king_moves\n");
  int num_moves = 0;
  while (kings) {
    int index = lsb_index(kings);
    pop_lsb(kings);

    Bitboard attacks = king_moves[index];
    Bitboard pseudo_moves = attacks & ~same_side_occupied;

    while (pseudo_moves) {
      int to_square = lsb_index(pseudo_moves);
      pop_lsb(pseudo_moves);
      if (enemy_occupied & (1ULL << to_square)) {
        *moves++ = encode_move(index, to_square, FLAGS_CAPTURE);
      } else {
        *moves++ = encode_move(index, to_square, 0);
      }
      num_moves++;
    }
  }

  return num_moves;
}

int generate_rook_moves(Bitboard rooks, Bitboard occupied,
                        Bitboard same_side_occupied, Bitboard enemy_occupied,
                        Move *moves) {
  DP_PRINTF("FUNC_TRACE", "generate_rook_moves\n");
  int num_moves = 0;

  while (rooks) {
    int index = lsb_index(rooks);
    pop_lsb(rooks);
    Bitboard rook_moves =
        get_rook_attack_board(index, occupied, same_side_occupied);

    while (rook_moves) {
      int to_square = lsb_index(rook_moves);
      pop_lsb(rook_moves);
      if (enemy_occupied & (1ULL << to_square)) {
        *moves++ = encode_move(index, to_square, FLAGS_CAPTURE);
      } else {
        *moves++ = encode_move(index, to_square, 0);
      }
      num_moves++;
    }
  }

  return num_moves;
}

int generate_bishop_moves(Bitboard bishops, Bitboard occupied,
                          Bitboard same_side_occupied, Bitboard enemy_occupied,
                          Move *moves) {
  DP_PRINTF("FUNC_TRACE", "generate_bishop_moves\n");
  int num_moves = 0;

  while (bishops) {
    int index = lsb_index(bishops);
    pop_lsb(bishops);
    Bitboard bishop_moves =
        get_bishop_attack_board(index, occupied, same_side_occupied);

    while (bishop_moves) {
      int to_square = lsb_index(bishop_moves);
      pop_lsb(bishop_moves);
      if (enemy_occupied & (1ULL << to_square)) {
        *moves++ = encode_move(index, to_square, FLAGS_CAPTURE);
      } else {
        *moves++ = encode_move(index, to_square, 0);
      }
      num_moves++;
    }
  }

  return num_moves;
}

static inline bool index_in_check(Board *board, ToMove color,
                                  int square_index) {
  DP_PRINTF("FUNC_TRACE", "index_in_check\n");
  Bitboard same_side_occupied = board->occupied_by_color[color];

  ToMove opposite_color = OPPOSITE_COLOR(color);
  Bitboard potential_rook_attacks =
      get_rook_attack_board(square_index, board->occupied, same_side_occupied);
  if (potential_rook_attacks & (board->pieces[OPPOSITE_COLOR(color)][ROOK] |
                                board->pieces[OPPOSITE_COLOR(color)][QUEEN])) {
    return true;
  }

  Bitboard potential_bishop_attacks = get_bishop_attack_board(
      square_index, board->occupied, same_side_occupied);
  if (potential_bishop_attacks & (board->pieces[opposite_color][BISHOP] |
                                  board->pieces[opposite_color][QUEEN])) {
    return true;
  }

  Bitboard potential_knight_attacks = knight_moves[square_index];
  if (potential_knight_attacks & board->pieces[opposite_color][KNIGHT]) {
    return true;
  }

  Bitboard potential_king_attacks = king_moves[square_index];
  if (potential_king_attacks & board->pieces[opposite_color][KING]) {
    return true;
  }

  Bitboard index_square = 1ULL << square_index;

  Bitboard potential_pawn_attacks =
      (color == WHITE) ? (index_square << 7) & ~FILE_A  // left capture
                       : (index_square >> 9) & ~FILE_A; // right capture

  potential_pawn_attacks |= (color == WHITE)
                                ? (index_square << 9) & ~FILE_H // right capture
                                : (index_square >> 7) & ~FILE_H; // left capture

  if (potential_pawn_attacks & board->pieces[opposite_color][PAWN]) {
    return true;
  }
  return false;
}

int generate_castling(Board *board, Move *moves, GameStateDetails details) {
  DP_PRINTF("FUNC_TRACE", "generate_castling\n");
  bool short_available = (board->to_move == WHITE)
                             ? (details.castling_rights & CR_WHITE_SHORT)
                             : (details.castling_rights & CR_BLACK_SHORT);
  bool long_available = (board->to_move == WHITE)
                            ? (details.castling_rights & CR_WHITE_LONG)
                            : (details.castling_rights & CR_BLACK_LONG);

  int num_moves = 0;

  if (short_available) {
    Bitboard clearance_mask = (board->to_move == WHITE)
                                  ? WHITE_SHORT_CASTLE_CLEARANCE_MASK
                                  : BLACK_SHORT_CASTLE_CLEARANCE_MASK;

    if ((board->occupied & clearance_mask) == 0) {
      Bitboard check_mask = (board->to_move == WHITE)
                                ? WHITE_SHORT_CASTLE_CHECK_MASK
                                : BLACK_SHORT_CASTLE_CHECK_MASK;

      bool check_cleared = true; // bars
      while (check_mask) {
        int index = lsb_index(check_mask);
        pop_lsb(check_mask);
        if (index_in_check(board, board->to_move, index)) {
          check_cleared = false;
          break;
        }
      }

      if (check_cleared) {
        *moves++ = encode_move(0, 0, FLAGS_SHORT_CASTLE);
        num_moves++;
      }
    }
  }
  if (long_available) {
    Bitboard clearance_mask = (board->to_move == WHITE)
                                  ? WHITE_LONG_CASTLE_CLEARANCE_MASK
                                  : BLACK_LONG_CASTLE_CLEARANCE_MASK;

    if ((board->occupied & clearance_mask) == 0) {
      Bitboard check_mask = (board->to_move == WHITE)
                                ? WHITE_LONG_CASTLE_CHECK_MASK
                                : BLACK_LONG_CASTLE_CHECK_MASK;

      bool check_cleared = true; // bars again
      while (check_mask) {
        int index = lsb_index(check_mask);
        pop_lsb(check_mask);
        if (index_in_check(board, board->to_move, index)) {
          check_cleared = false;
          break;
        }
      }

      if (check_cleared) {
        *moves++ = encode_move(0, 0, FLAGS_LONG_CASTLE);
        num_moves++;
      }
    }
  }
  return num_moves;
}

int generate_moves(Board *board, Move moves[MAX_MOVES]) {
  DP_PRINTF("FUNC_TRACE", "generate_moves\n");
  int move_count = 0;
  if (board->game_state != GS_ONGOING) {
    return 0;
  }

  GameStateDetails gsd = BOARD_CURR_STATE(board);

  Bitboard pawns = board->pieces[board->to_move][PAWN];
  Bitboard knights = board->pieces[board->to_move][KNIGHT];
  Bitboard bishops = board->pieces[board->to_move][BISHOP];
  Bitboard rooks = board->pieces[board->to_move][ROOK];
  Bitboard queens = board->pieces[board->to_move][QUEEN];
  Bitboard kings = board->pieces[board->to_move][KING];
  Bitboard same_side_occupied = board->occupied_by_color[board->to_move];
  Bitboard enemy_side_occupied = board->occupied_by_color[1 - board->to_move];

  move_count += generate_pawn_captures(pawns, enemy_side_occupied,
                                       moves + move_count, board->to_move);
  move_count += generate_en_passant(pawns, gsd.en_passant, board->to_move,
                                    moves + move_count);

  move_count += generate_castling(board, moves + move_count, gsd);

  move_count += generate_pawn_pushes(pawns, board->occupied, moves + move_count,
                                     board->to_move);
  move_count += generate_knight_moves(knights, same_side_occupied,
                                      enemy_side_occupied, moves + move_count);
  move_count +=
      generate_rook_moves(rooks | queens, board->occupied, same_side_occupied,
                          enemy_side_occupied, moves + move_count);
  move_count += generate_bishop_moves(bishops | queens, board->occupied,
                                      same_side_occupied, enemy_side_occupied,
                                      moves + move_count);
  move_count += generate_king_moves(kings, same_side_occupied,
                                    enemy_side_occupied, moves + move_count);
  return move_count;
}

bool king_in_check(Board *board, ToMove color) {
  return index_in_check(board, color, lsb_index(board->pieces[color][KING]));
}

char *move_to_string(Move move) {
  static char buffer[16];
  int from_square = move_from_square(move);
  int to_square = move_to_square(move);
  int flags = move_flags(move);

  snprintf(buffer, sizeof(buffer), "%c%c-%c%c, %d", 'h' - (from_square % 8),
           '1' + (from_square / 8), 'h' - (to_square % 8),
           '1' + (to_square / 8), flags);

  return buffer;
}

static inline int generate_flags(Board *board, int start_index, int end_index,
                                 char promo) {
  Bitboard start_bb = (1ULL << start_index);
  Bitboard end_bb = (1ULL << end_index);
  if (board->pieces[board->to_move][PAWN] & start_bb) {
    if ((BOARD_CURR_STATE(board).en_passant & end_bb)) {
      return FLAGS_EN_PASSANT;
    }
    int double_push_offset = (board->to_move == WHITE) ? 16 : -16;
    if (start_index + double_push_offset == end_index) {
      return FLAGS_DOUBLE_PUSH;
    }
  }
  int out_flags = FLAGS_NONE;
  // clang-format off
  if      (promo == 'n') { out_flags |= FLAGS_KNIGHT_PROMOTION; }
  else if (promo == 'b') { out_flags |= FLAGS_BISHOP_PROMOTION; }
  else if (promo == 'r') { out_flags |= FLAGS_ROOK_PROMOTION;   }
  else if (promo == 'q') { out_flags |= FLAGS_QUEEN_PROMOTION;  }

  if (board->piece_table[end_index] != PIECE_NONE) {
    out_flags |= FLAGS_CAPTURE;
  }
  // clang-format on
  return out_flags;
}

Move algebraic_to_move(Board *board, const char *str) {
  size_t str_len = strlen(str);
  if (str == NULL || str_len < 4) {
    return NULL_MOVE; // Invalid move string
  }

  if (or_strcmp(str, 2, "e1g1", "e8g8")) {
    return FLAGS_SHORT_CASTLE;
  } else if (or_strcmp(str, 2, "e1c1", "e8c8")) {
    return FLAGS_LONG_CASTLE;
  }

  int start_file = str[0] - 'a';
  int start_rank = str[1] - '1';
  int end_file = str[2] - 'a';
  int end_rank = str[3] - '1';

  int start_index = start_rank * 8 + (7 - start_file);
  int end_index = end_rank * 8 + (7 - end_file);

  int flags = generate_flags(board, start_index, end_index, str[4]);
  return encode_move(start_index, end_index, flags);
}

char *move_to_algebraic(Move move, ToMove color) {
  if (move == NULL_MOVE) {
    return "0000";
  }
  if (move == 0) {
    return "0000";
  }

  int start_index = move_from_square(move);
  int end_index = move_to_square(move);
  int flags = move_flags(move);
  if (flags == FLAGS_SHORT_CASTLE) {
    return (color == WHITE) ? "e1g1" : "e8g8";
  } else if (flags == FLAGS_LONG_CASTLE) {
    return (color == WHITE) ? "e1c1" : "e8c8";
  }

  static char out_buffer[6] = {0};
  int start_file = start_index % 8;
  int start_rank = start_index / 8;
  int end_file = end_index % 8;
  int end_rank = end_index / 8;

  out_buffer[0] = 'a' + (7 - start_file);
  out_buffer[1] = '1' + start_rank;
  out_buffer[2] = 'a' + (7 - end_file);
  out_buffer[3] = '1' + end_rank;

  int promo_flags = flags & PROMOTION_MASK;

  if (promo_flags == FLAGS_KNIGHT_PROMOTION) {
    out_buffer[4] = 'n';
  } else if (promo_flags == FLAGS_BISHOP_PROMOTION) {
    out_buffer[4] = 'b';
  } else if (promo_flags == FLAGS_ROOK_PROMOTION) {
    out_buffer[4] = 'r';
  } else if (promo_flags == FLAGS_QUEEN_PROMOTION) {
    out_buffer[4] = 'q';
  } else {
    out_buffer[4] = '\0';
  }

  return out_buffer;
}

bool is_capture(Move move) {
  int flags = move_flags(move);
  return (flags & FLAGS_CAPTURE) && flags != FLAGS_LONG_CASTLE;
}

int is_promotion(Move move) {
  int flags = move_flags(move);

  switch (flags & PROMOTION_MASK) {
  case FLAGS_QUEEN_PROMOTION:
    return 4;
  case FLAGS_ROOK_PROMOTION:
    return 3;
  case FLAGS_BISHOP_PROMOTION:
    return 2;
  case FLAGS_KNIGHT_PROMOTION:
    return 1;
  default:
    return 0;
  }
}

Bitboard generate_attacks(Board *pos, Piece piece, int square, ToMove color) {
  switch (piece) {
  case PAWN:
    assert(false);
    return 0; // Pawns handled separately
  case KNIGHT:
    return knight_moves[square] & ~(pos->occupied_by_color[color]);
  case BISHOP:
    return get_bishop_attack_board(square, pos->occupied,
                                   pos->occupied_by_color[color]);
  case ROOK:
    return get_rook_attack_board(square, pos->occupied,
                                 pos->occupied_by_color[color]);
  case QUEEN:
    return get_bishop_attack_board(square, pos->occupied,
                                   pos->occupied_by_color[color]) |
           get_rook_attack_board(square, pos->occupied,
                                 pos->occupied_by_color[color]);
  case KING:
    return king_moves[square] & ~(pos->occupied_by_color[color]);
  }
  return 0ULL;
}
