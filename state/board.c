//
// Created by Major Zangari on 4/19/25.
//

#include "board.h"
#include "bithelpers.h"
#include "debug_printer.h"
#include "fen.h"
#include "hash.h"
#include "move.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

Board *init_default_board() {
  return fen_to_board(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

static inline void check_rook_castling_right_change(Board *board,
                                                    GameStateDetails *state,
                                                    uint8_t square) {
  if (square == 0) {
    state->castling_rights &= ~CR_WHITE_SHORT;
    state->hash = toggle_castling_rights(state->hash, ZOBRIST_CR_WHITE_SHORT);
  } else if (square == 7) {
    state->castling_rights &= ~CR_WHITE_LONG;
    state->hash = toggle_castling_rights(state->hash, ZOBRIST_CR_WHITE_LONG);
  } else if (square == 56) {
    state->castling_rights &= ~CR_BLACK_SHORT;
    state->hash = toggle_castling_rights(state->hash, ZOBRIST_CR_BLACK_SHORT);
  } else if (square == 63) {
    state->castling_rights &= ~CR_BLACK_LONG;
    state->hash = toggle_castling_rights(state->hash, ZOBRIST_CR_BLACK_LONG);
  }
}

// TODO: bloat?
static inline void manage_move_castling_rights(Board *board,
                                               GameStateDetails *state,
                                               uint8_t from_square) {
  Piece piece = board->piece_table[from_square];
  ToMove color = board->to_move;

  if (piece == ROOK) {
    check_rook_castling_right_change(board, state, from_square);
  } else if (piece == KING) {
    if (color == WHITE) {
      state->castling_rights &= ~(CR_WHITE_SHORT | CR_WHITE_LONG);
      state->hash = toggle_castling_rights(state->hash, ZOBRIST_CR_WHITE_SHORT);
      state->hash = toggle_castling_rights(state->hash, ZOBRIST_CR_WHITE_LONG);
    } else {
      state->castling_rights &= ~(CR_BLACK_SHORT | CR_BLACK_LONG);
      state->hash = toggle_castling_rights(state->hash, ZOBRIST_CR_BLACK_SHORT);
      state->hash = toggle_castling_rights(state->hash, ZOBRIST_CR_BLACK_LONG);
    }
  }
}

static inline void move_piece(Board *board, GameStateDetails *state,
                              uint8_t from_square, uint8_t to_square,
                              uint8_t flags) {
  Piece piece = board->piece_table[from_square];
  ToMove color = board->to_move;

  manage_move_castling_rights(board, state, from_square);

  board->pieces[color][piece] &= ~(1ULL << from_square);
  board->pieces[color][piece] |= (1ULL << to_square);
  board->occupied_by_color[color] &= ~(1ULL << from_square);
  board->occupied_by_color[color] |= (1ULL << to_square);
  board->occupied &= ~(1ULL << from_square);
  board->occupied |= (1ULL << to_square);

  Piece previous_piece = board->piece_table[to_square];
  if (flags & FLAGS_CAPTURE) {
    ToMove captured_color = OPPOSITE_COLOR(color);
    board->pieces[captured_color][previous_piece] &= ~(1ULL << to_square);
    board->occupied_by_color[captured_color] &= ~(1ULL << to_square);
    state->captured_piece = previous_piece;
    state->halfmove_clock = 0;
    state->hash =
        toggle_piece(state->hash, previous_piece, to_square, captured_color);
    if (previous_piece == ROOK) {
      check_rook_castling_right_change(board, state, to_square);
    }
  }
  board->piece_table[to_square] = piece;
  board->piece_table[from_square] = PIECE_NONE;
  state->hash =
      move_piece_hash(state->hash, piece, from_square, to_square, color);
}

// SHOULD BE USED AFTER POPPING THE STATE
static inline void undo_move_piece(Board *board, uint8_t to_square,
                                   uint8_t from_square, uint8_t flags,
                                   Piece previous_piece) {
  Piece piece = board->piece_table[to_square];
  ToMove color = OPPOSITE_COLOR(board->to_move);

  board->pieces[color][piece] &= ~(1ULL << to_square);
  board->pieces[color][piece] |= (1ULL << from_square);
  board->occupied_by_color[color] &= ~(1ULL << to_square);
  board->occupied_by_color[color] |= (1ULL << from_square);
  board->occupied |= (1ULL << from_square);

  if (previous_piece != PIECE_NONE) {
    ToMove captured_color = OPPOSITE_COLOR(color);
    board->pieces[captured_color][previous_piece] |= (1ULL << to_square);
    board->occupied_by_color[captured_color] |= (1ULL << to_square);
    board->piece_table[to_square] = previous_piece;
  } else {
    board->occupied &= ~(1ULL << to_square);
  }
  board->piece_table[to_square] = previous_piece;
  board->piece_table[from_square] = piece;
}

// CAN NOT BE USED FOR CAPTURES
static inline void set_piece(Board *board, GameStateDetails *state, int square,
                             Piece piece) {
  ToMove color = board->to_move;
  Piece previous_piece = board->piece_table[square];
  if (previous_piece != PIECE_NONE) {
    board->pieces[color][previous_piece] &= ~(1ULL << square);
    state->hash = toggle_piece(state->hash, previous_piece, square, color);
    board->occupied_by_color[color] &= ~(1ULL << square);
  }
  board->pieces[color][piece] |= (1ULL << square);
  board->piece_table[square] = piece;
  state->hash = toggle_piece(state->hash, piece, square, color);
  board->occupied_by_color[color] |= (1ULL << square);
  board->occupied |= (1ULL << square);
}

// CAN NOT BE USED FOR CAPTURES, POP STATE FIRST
static inline void set_piece_no_hash(Board *board, int square, Piece piece,
                                     ToMove color) {
  Piece previous_piece = board->piece_table[square];
  if (previous_piece != PIECE_NONE) {
    board->pieces[color][previous_piece] &= ~(1ULL << square);
  }
  board->piece_table[square] = piece;
  board->pieces[color][piece] |= (1ULL << square);
  board->occupied_by_color[color] |= (1ULL << square);
  board->occupied |= (1ULL << square);
}

// checks if board is in a repetition, 50-move rule, or TODO: insufficient
// material
static inline bool is_special_draw(Board *board) {
  ToMove opposite_color = OPPOSITE_COLOR(board->to_move);
  GameStateDetails curr_state = BOARD_CURR_STATE(board);

  // 50-move rule
  if (curr_state.halfmove_clock >= 100) {
    return true;
  }

  // repetition
  int identical_states = 1;
  for (int i = 0;
       i < curr_state.halfmove_clock && (board->game_state_stack.top - i >= 0);
       i++) {
    GameStateDetails previous_state =
        GameStateDetailsStack_peek_down(&board->game_state_stack, i);
    if (previous_state.hash == curr_state.hash) {
      identical_states++;
    }
    if (identical_states >= 3) {
      return true;
    }
  }

  // TODO: insufficient material
  return false;
}

static inline void short_castle(Board *board, GameStateDetails *new_state) {
  uint64_t current_hash = new_state->hash;
  if (board->to_move == WHITE) {
    board->pieces[WHITE][KING] ^= 0xAULL;
    board->pieces[WHITE][ROOK] ^= 0x5ULL;
    board->occupied_by_color[WHITE] ^= 0xAULL | 0x5ULL;
    board->occupied ^= 0xAULL | 0x5ULL;

    board->piece_table[0] = PIECE_NONE;
    board->piece_table[1] = KING;
    board->piece_table[2] = ROOK;
    board->piece_table[3] = PIECE_NONE;

    current_hash = move_piece_hash(current_hash, KING, 3, 1, WHITE);
    current_hash = move_piece_hash(current_hash, ROOK, 0, 2, WHITE);
    current_hash = toggle_castling_rights(current_hash, ZOBRIST_CR_WHITE_SHORT);
    if (new_state->castling_rights & CR_WHITE_LONG) {
      current_hash =
          toggle_castling_rights(current_hash, ZOBRIST_CR_WHITE_LONG);
    }

    new_state->castling_rights &= ~(CR_WHITE_SHORT | CR_WHITE_LONG);
  } else {
    board->pieces[BLACK][KING] ^= 0xA00000000000000ULL;
    board->pieces[BLACK][ROOK] ^= 0x500000000000000ULL;
    board->occupied_by_color[BLACK] ^=
        0xA00000000000000ULL | 0x500000000000000ULL;
    board->occupied ^= 0xA00000000000000ULL | 0x500000000000000ULL;

    board->piece_table[56] = PIECE_NONE;
    board->piece_table[57] = KING;
    board->piece_table[58] = ROOK;
    board->piece_table[59] = PIECE_NONE;

    current_hash = move_piece_hash(current_hash, KING, 59, 57, BLACK);
    current_hash = move_piece_hash(current_hash, ROOK, 56, 58, BLACK);
    current_hash = toggle_castling_rights(current_hash, ZOBRIST_CR_BLACK_SHORT);
    if (new_state->castling_rights & CR_BLACK_LONG) {
      current_hash =
          toggle_castling_rights(current_hash, ZOBRIST_CR_BLACK_LONG);
    }

    new_state->castling_rights &= ~(CR_BLACK_SHORT | CR_BLACK_LONG);
  }
  new_state->hash = current_hash;
  new_state->halfmove_clock = 0;
}

static inline void reverse_short_castle(Board *board) {
  // reversing white's castle
  if (board->to_move == BLACK) {
    board->pieces[WHITE][KING] ^= 0xAULL;
    board->pieces[WHITE][ROOK] ^= 0x5ULL;
    board->occupied_by_color[WHITE] ^= 0xAULL | 0x5ULL;
    board->occupied ^= 0xAULL | 0x5ULL;

    board->piece_table[0] = ROOK;
    board->piece_table[1] = PIECE_NONE;
    board->piece_table[2] = PIECE_NONE;
    board->piece_table[3] = KING;
  } else {
    board->pieces[BLACK][KING] ^= 0xA00000000000000ULL;
    board->pieces[BLACK][ROOK] ^= 0x500000000000000ULL;
    board->occupied_by_color[BLACK] ^=
        0xA00000000000000ULL | 0x500000000000000ULL;
    board->occupied ^= 0xA00000000000000ULL | 0x500000000000000ULL;

    board->piece_table[56] = ROOK;
    board->piece_table[57] = PIECE_NONE;
    board->piece_table[58] = PIECE_NONE;
    board->piece_table[59] = KING;
  }
}

static inline void long_castle(Board *board, GameStateDetails *new_state) {
  uint64_t current_hash = new_state->hash;

  if (board->to_move == WHITE) {
    board->pieces[WHITE][KING] ^= 0x28ULL;
    board->pieces[WHITE][ROOK] ^= 0x90ULL;
    board->occupied_by_color[WHITE] ^= 0x28ULL | 0x90ULL;
    board->occupied ^= 0x28ULL | 0x90ULL;

    board->piece_table[3] = PIECE_NONE;
    board->piece_table[4] = ROOK;
    board->piece_table[5] = KING;
    board->piece_table[7] = PIECE_NONE;

    current_hash = move_piece_hash(current_hash, KING, 3, 5, WHITE);
    current_hash = move_piece_hash(current_hash, ROOK, 7, 4, WHITE);
    current_hash = toggle_castling_rights(current_hash, ZOBRIST_CR_WHITE_LONG);
    if (new_state->castling_rights & CR_WHITE_SHORT) {
      current_hash =
          toggle_castling_rights(current_hash, ZOBRIST_CR_WHITE_SHORT);
    }

    new_state->castling_rights &= ~(CR_WHITE_SHORT | CR_WHITE_LONG);
  } else {
    board->pieces[BLACK][KING] ^= 0x2800000000000000ULL;
    board->pieces[BLACK][ROOK] ^= 0x9000000000000000ULL;
    board->occupied_by_color[BLACK] ^=
        0x2800000000000000ULL | 0x9000000000000000ULL;
    board->occupied ^= 0x2800000000000000ULL | 0x9000000000000000ULL;

    board->piece_table[59] = PIECE_NONE;
    board->piece_table[60] = ROOK;
    board->piece_table[61] = KING;
    board->piece_table[63] = PIECE_NONE;

    current_hash = move_piece_hash(current_hash, KING, 59, 61, BLACK);
    current_hash = move_piece_hash(current_hash, ROOK, 63, 60, BLACK);
    current_hash = toggle_castling_rights(current_hash, ZOBRIST_CR_BLACK_LONG);
    if (new_state->castling_rights & CR_BLACK_SHORT) {
      current_hash =
          toggle_castling_rights(current_hash, ZOBRIST_CR_BLACK_SHORT);
    }
    new_state->castling_rights &= ~(CR_BLACK_SHORT | CR_BLACK_LONG);
  }
  new_state->halfmove_clock = 0;
}

static inline void reverse_long_castle(Board *board) {
  // reversing white's castle
  if (board->to_move == BLACK) {
    board->pieces[WHITE][KING] ^= 0x28ULL;
    board->pieces[WHITE][ROOK] ^= 0x90ULL;
    board->occupied_by_color[WHITE] ^= 0x28ULL | 0x90ULL;
    board->occupied ^= 0x28ULL | 0x90ULL;

    board->piece_table[3] = KING;
    board->piece_table[4] = PIECE_NONE;
    board->piece_table[5] = PIECE_NONE;
    board->piece_table[7] = ROOK;
  } else {
    board->pieces[BLACK][KING] ^= 0x2800000000000000ULL;
    board->pieces[BLACK][ROOK] ^= 0x9000000000000000ULL;
    board->occupied_by_color[BLACK] ^=
        0x2800000000000000ULL | 0x9000000000000000ULL;
    board->occupied ^= 0x2800000000000000ULL | 0x9000000000000000ULL;

    board->piece_table[59] = KING;
    board->piece_table[60] = PIECE_NONE;
    board->piece_table[61] = PIECE_NONE;
    board->piece_table[63] = ROOK;
  }
}

void kill_piece(Board *board, GameStateDetails *state, uint8_t square,
                ToMove color) {
  DP_PRINTF("FUNC_TRACE", "kill_piece\n");
  Piece piece = board->piece_table[square];
  if (piece == PIECE_NONE) {
    return; // nothing to kill
  }
  state->captured_piece = piece;
  board->pieces[color][piece] &= ~(1ULL << square);
  board->occupied_by_color[color] &= ~(1ULL << square);
  board->occupied &= ~(1ULL << square);
  board->piece_table[square] = PIECE_NONE;
  state->hash = toggle_piece(state->hash, piece, square, color);
}

void board_make_move(Board *board, Move move) {
  DP_PRINTF("FUNC_TRACE", "board_make_move\n");
  if (move == NULL_MOVE) {
    return;
  }

  uint8_t from_square = move_from_square(move);
  uint8_t to_square = move_to_square(move);
  uint8_t flags = move_flags(move);

  GameStateDetails new_state = {0}; // TODO: push and then edit to save copy
  new_state.halfmove_clock = BOARD_CURR_STATE(board).halfmove_clock + 1;
  new_state.castling_rights = BOARD_CURR_STATE(board).castling_rights;
  new_state.captured_piece = PIECE_NONE;
  new_state.hash = toggle_turn(BOARD_CURR_STATE(board).hash);

  int old_ep = lsb_index(BOARD_CURR_STATE(board).en_passant);
  new_state.hash = toggle_en_passant(new_state.hash, old_ep);

  switch (flags) {
  case FLAGS_PAWN_PUSH:
    move_piece(board, &new_state, from_square, to_square, flags);
    new_state.halfmove_clock = 0;
    break;
  case FLAGS_DOUBLE_PUSH:
    move_piece(board, &new_state, from_square, to_square, flags);
    new_state.halfmove_clock = 0;
    int ep_shift =
        (board->to_move == WHITE) ? (to_square - 8) : (to_square + 8);
    new_state.en_passant = 1ULL << ep_shift;
    new_state.hash = toggle_en_passant(new_state.hash, new_state.en_passant);
    break;
  case FLAGS_SHORT_CASTLE:
    short_castle(board, &new_state);
    break;
  case FLAGS_LONG_CASTLE:
    long_castle(board, &new_state);
    break;
  case FLAGS_EN_PASSANT: // why is this a case? TODO: figure out
    move_piece(board, &new_state, from_square, to_square, FLAGS_NONE);
    int ep_capture_shift =
        (board->to_move == WHITE) ? (to_square - 8) : (to_square + 8);
    kill_piece(board, &new_state, ep_capture_shift,
               OPPOSITE_COLOR(board->to_move));

    break;
  default:
    move_piece(board, &new_state, from_square, to_square, flags);

    // clang-format off
    if      ((flags & PROMOTION_MASK) == FLAGS_KNIGHT_PROMOTION) { set_piece(board, &new_state, to_square, KNIGHT); }
    else if ((flags & PROMOTION_MASK) == FLAGS_BISHOP_PROMOTION) { set_piece(board, &new_state, to_square, BISHOP); }
    else if ((flags & PROMOTION_MASK) == FLAGS_ROOK_PROMOTION  ) { set_piece(board, &new_state, to_square, ROOK);   }
    else if ((flags & PROMOTION_MASK) == FLAGS_QUEEN_PROMOTION ) { set_piece(board, &new_state, to_square, QUEEN);  }
    // clang-format on
    break;
  }
  GameStateDetailsStack_push(&board->game_state_stack, new_state);
  if (is_special_draw(board)) {
    board->game_state = GS_DRAW;
  } else {
    board->game_state = GS_ONGOING; // technically could be won but saves time
                                    // to not figure out yet
  }
  board->to_move = OPPOSITE_COLOR(board->to_move);
}

void board_unmake_move(Board *board, Move move) {
  DP_PRINTF("FUNC_TRACE", "board_unmake_move\n");
  if (move == NULL_MOVE) {
    return;
  }

  GameStateDetails old_state =
      GameStateDetailsStack_pop(&board->game_state_stack);

  Piece captured_piece = old_state.captured_piece;

  uint8_t from_square = move_from_square(move);
  uint8_t to_square = move_to_square(move);
  uint8_t flags = move_flags(move);

  switch (flags) {
  case FLAGS_PAWN_PUSH:
  case FLAGS_DOUBLE_PUSH:
    undo_move_piece(board, to_square, from_square, flags, captured_piece);
    break;
  case FLAGS_EN_PASSANT:
    undo_move_piece(board, to_square, from_square, FLAGS_NONE, PIECE_NONE);
    int ep_capture_shift =
        (board->to_move == WHITE) ? (to_square + 8) : (to_square - 8);
    set_piece_no_hash(board, ep_capture_shift, PAWN, board->to_move);
    break;
  case FLAGS_SHORT_CASTLE:
    reverse_short_castle(board);
    break;
  case FLAGS_LONG_CASTLE:
    reverse_long_castle(board);
    break;
  default:
    undo_move_piece(board, to_square, from_square, flags, captured_piece);

    // clang-format off
    if      ((flags & PROMOTION_MASK) == FLAGS_KNIGHT_PROMOTION) { set_piece_no_hash(board, from_square, PAWN, OPPOSITE_COLOR(board->to_move)); }
    else if ((flags & PROMOTION_MASK) == FLAGS_BISHOP_PROMOTION) { set_piece_no_hash(board, from_square, PAWN, OPPOSITE_COLOR(board->to_move)); }
    else if ((flags & PROMOTION_MASK) == FLAGS_ROOK_PROMOTION  ) { set_piece_no_hash(board, from_square, PAWN, OPPOSITE_COLOR(board->to_move)); }
    else if ((flags & PROMOTION_MASK) == FLAGS_QUEEN_PROMOTION ) { set_piece_no_hash(board, from_square, PAWN, OPPOSITE_COLOR(board->to_move)); }
    // clang-format on

    break;
  }
  board->game_state = GS_ONGOING;
  board->to_move = OPPOSITE_COLOR(board->to_move);
}

bool board_valid(Board *board) {
  DP_PRINTF("FUNC_TRACE", "board_valid\n");
  uint64_t all_pieces = 0;
  for (int color = 0; color < 2; color++) {
    for (int piece = 0; piece < 6; piece++) {
      if (all_pieces & board->pieces[color][piece]) {
        printf("Duplicate piece found: color %d, piece %d\n", color, piece);
        return false;
      }
      all_pieces |= board->pieces[color][piece];
    }
  }
  if (all_pieces != board->occupied) {
    printf("Occupied bitboard does not match pieces bitboards: all_pieces:%lx, "
           "occupied:%lx.\n",
           all_pieces, board->occupied);
    return false;
  }
  return true;
}

void board_update_occupied(Board *board) {
  board->occupied_by_color[WHITE] = 0;
  board->occupied_by_color[BLACK] = 0;
  board->occupied = 0;

  for (int piece = 0; piece < 6; piece++) {
    board->occupied_by_color[WHITE] |= board->pieces[WHITE][piece];
    board->occupied_by_color[BLACK] |= board->pieces[BLACK][piece];
  }

  board->occupied =
      board->occupied_by_color[WHITE] | board->occupied_by_color[BLACK];
}

void board_update_piece_table(Board *board) {
  for (size_t i = 0; i < 63; i++) {
    board->piece_table[i] = PIECE_NONE;
  }

  for (int color = 0; color < 2; color++) {
    for (int piece = 0; piece < 6; piece++) {
      Bitboard occ = board->pieces[color][piece];
      while (occ) {
        uint8_t square = lsb_index(occ);
        board->piece_table[square] = piece;
        pop_lsb(occ);
      }
    }
  }
}
