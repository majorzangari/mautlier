//
// Created by Major Zangari on 4/19/25.
//

#include "board.h"
#include "bithelpers.h"
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

static inline void move_piece(Board *board, GameStateDetails *state,
                              uint8_t from_square, uint8_t to_square,
                              uint8_t flags) {
  Piece piece = board->piece_table[from_square];
  ToMove color = board->to_move;
  board->pieces[color][piece] &= ~(1ULL << from_square);
  board->pieces[color][piece] |= (1ULL << to_square);
  board->occupied_by_color[color] &= ~(1ULL << from_square);
  board->occupied_by_color[color] |= (1ULL << to_square);
  board->occupied &= ~(1ULL << from_square);
  board->occupied |= (1ULL << to_square);

  Piece previous_piece = board->piece_table[to_square];
  if (flags & FLAGS_CAPTURE) {
    ToMove captured_color = color == WHITE ? BLACK : WHITE;
    board->pieces[captured_color][previous_piece] &= ~(1ULL << to_square);
    board->occupied_by_color[captured_color] &= ~(1ULL << to_square);
    state->captured_piece = previous_piece;
    state->halfmove_clock = 0;
    state->hash =
        toggle_piece(state->hash, previous_piece, to_square, captured_color);
  }
  board->piece_table[to_square] = piece;
  board->piece_table[from_square] = PIECE_NONE;
  state->hash =
      move_piece_hash(state->hash, piece, from_square, to_square, color);
}

// SHOULD BE USED AFTER POPPING THE STATE
static inline void undo_move_piece(Board *board, uint8_t to_square,
                                   uint8_t from_square, uint8_t flags) {
  Piece piece = board->piece_table[to_square];
  Piece previous_piece = BOARD_CURR_STATE(board).captured_piece;
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

// does not set captured piece
static inline void set_piece(Board *board, GameStateDetails *state, int square,
                             Piece piece) {
  ToMove color = board->to_move;
  Piece previous_piece = board->piece_table[square];
  if (previous_piece != PIECE_NONE) {
    board->pieces[OPPOSITE_COLOR(color)][previous_piece] &= ~(1ULL << square);
    state->hash = toggle_piece(state->hash, previous_piece, square, color);
    board->occupied_by_color[OPPOSITE_COLOR(color)] &= ~(1ULL << square);
  }
  board->pieces[color][piece] |= (1ULL << square);
  board->piece_table[square] = piece;
  state->hash = toggle_piece(state->hash, piece, square, color);
  board->occupied_by_color[color] |= (1ULL << square);
  board->occupied |= (1ULL << square);
}

// hacky? basically a copy paste TODO: fix
static inline void set_piece_no_hash(Board *board, int square, Piece piece) {
  ToMove color = board->to_move;
  Piece previous_piece = board->piece_table[square];
  if (previous_piece != PIECE_NONE) {
    board->pieces[OPPOSITE_COLOR(color)][previous_piece] &= ~(1ULL << square);
    board->occupied_by_color[OPPOSITE_COLOR(color)] &= ~(1ULL << square);
  }
  board->pieces[color][piece] |= (1ULL << square);
  board->piece_table[square] = piece;
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
  for (int i = 1; i <= curr_state.halfmove_clock; i++) {
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
    board->pieces[WHITE][KING] |= 0xAULL;
    board->pieces[WHITE][ROOK] |= 0x5ULL;
    board->occupied_by_color[WHITE] |= 0xAULL | 0x5ULL;
    board->occupied |= 0xAULL | 0x5ULL;

    board->piece_table[0] = PIECE_NONE;
    board->piece_table[1] = KING;
    board->piece_table[2] = ROOK;
    board->piece_table[3] = PIECE_NONE;

    current_hash = move_piece_hash(current_hash, KING, 3, 1, WHITE);
    current_hash = move_piece_hash(current_hash, ROOK, 0, 2, WHITE);
    current_hash = toggle_castling_rights(current_hash, CR_WHITE_SHORT);
    if (new_state->castling_rights & CR_WHITE_LONG) {
      current_hash = toggle_castling_rights(current_hash, CR_WHITE_LONG);
    }

    new_state->castling_rights &= ~(CR_WHITE_SHORT | CR_WHITE_LONG);
  } else {
    board->pieces[BLACK][KING] |= 0xA00000000000000ULL;
    board->pieces[BLACK][ROOK] |= 0x500000000000000ULL;
    board->occupied_by_color[BLACK] |=
        0xA00000000000000ULL | 0x500000000000000ULL;
    board->occupied |= 0xA00000000000000ULL | 0x500000000000000ULL;

    board->piece_table[56] = PIECE_NONE;
    board->piece_table[57] = KING;
    board->piece_table[58] = ROOK;
    board->piece_table[59] = PIECE_NONE;

    current_hash = move_piece_hash(current_hash, KING, 59, 57, BLACK);
    current_hash = move_piece_hash(current_hash, ROOK, 56, 58, BLACK);
    current_hash = toggle_castling_rights(current_hash, CR_BLACK_SHORT);
    if (new_state->castling_rights & CR_BLACK_LONG) {
      current_hash = toggle_castling_rights(current_hash, CR_BLACK_LONG);
    }

    new_state->castling_rights &= ~(CR_BLACK_SHORT | CR_BLACK_LONG);
  }
  new_state->hash = current_hash;
  new_state->halfmove_clock = 0;
}

static inline void reverse_short_castle(Board *board) {
  if (board->to_move == WHITE) {
    board->pieces[WHITE][KING] |= 0xAULL;
    board->pieces[WHITE][ROOK] |= 0x5ULL;
    board->occupied_by_color[WHITE] |= 0xAULL | 0x5ULL;
    board->occupied |= 0xAULL | 0x5ULL;

    board->piece_table[0] = ROOK;
    board->piece_table[1] = PIECE_NONE;
    board->piece_table[2] = PIECE_NONE;
    board->piece_table[3] = KING;
  } else {
    board->pieces[BLACK][KING] |= 0xA00000000000000ULL;
    board->pieces[BLACK][ROOK] |= 0x500000000000000ULL;
    board->occupied_by_color[BLACK] |=
        0xA00000000000000ULL | 0x500000000000000ULL;
    board->occupied |= 0xA00000000000000ULL | 0x500000000000000ULL;

    board->piece_table[56] = ROOK;
    board->piece_table[57] = PIECE_NONE;
    board->piece_table[58] = PIECE_NONE;
    board->piece_table[59] = KING;
  }
}

static inline void long_castle(Board *board, GameStateDetails *new_state) {
  uint64_t current_hash = new_state->hash;

  if (board->to_move == WHITE) {
    board->pieces[WHITE][KING] |= 0x28ULL;
    board->pieces[WHITE][ROOK] |= 0x90ULL;
    board->occupied_by_color[WHITE] |= 0x28ULL | 0x90ULL;
    board->occupied |= 0x28ULL | 0x90ULL;

    board->piece_table[3] = PIECE_NONE;
    board->piece_table[4] = ROOK;
    board->piece_table[5] = KING;
    board->piece_table[7] = PIECE_NONE;

    current_hash = move_piece_hash(current_hash, KING, 3, 5, WHITE);
    current_hash = move_piece_hash(current_hash, ROOK, 7, 4, WHITE);
    current_hash = toggle_castling_rights(current_hash, CR_WHITE_LONG);
    if (new_state->castling_rights & CR_WHITE_SHORT) {
      current_hash = toggle_castling_rights(current_hash, CR_WHITE_SHORT);
    }

    new_state->castling_rights &= ~(CR_WHITE_SHORT | CR_WHITE_LONG);
  } else {
    board->pieces[BLACK][KING] |= 0x2800000000000000ULL;
    board->pieces[BLACK][ROOK] |= 0x9000000000000000ULL;
    board->occupied_by_color[BLACK] |=
        0x2800000000000000ULL | 0x9000000000000000ULL;
    board->occupied |= 0x2800000000000000ULL | 0x9000000000000000ULL;

    board->piece_table[59] = PIECE_NONE;
    board->piece_table[60] = ROOK;
    board->piece_table[61] = KING;
    board->piece_table[63] = PIECE_NONE;

    current_hash = move_piece_hash(current_hash, KING, 59, 61, BLACK);
    current_hash = move_piece_hash(current_hash, ROOK, 63, 60, BLACK);
    current_hash = toggle_castling_rights(current_hash, CR_BLACK_LONG);
    if (new_state->castling_rights & CR_BLACK_SHORT) {
      current_hash = toggle_castling_rights(current_hash, CR_BLACK_SHORT);
    }
    new_state->castling_rights &= ~(CR_BLACK_SHORT | CR_BLACK_LONG);
  }
  new_state->halfmove_clock = 0;
}

static inline void reverse_long_castle(Board *board) {
  if (board->to_move == WHITE) {
    board->pieces[WHITE][KING] |= 0x28ULL;
    board->pieces[WHITE][ROOK] |= 0x90ULL;
    board->occupied_by_color[WHITE] |= 0x28ULL | 0x90ULL;
    board->occupied |= 0x28ULL | 0x90ULL;

    board->piece_table[3] = KING;
    board->piece_table[4] = PIECE_NONE;
    board->piece_table[5] = PIECE_NONE;
    board->piece_table[7] = ROOK;
  } else {
    board->pieces[BLACK][KING] |= 0x2800000000000000ULL;
    board->pieces[BLACK][ROOK] |= 0x9000000000000000ULL;
    board->occupied_by_color[BLACK] |=
        0x2800000000000000ULL | 0x9000000000000000ULL;
    board->occupied |= 0x2800000000000000ULL | 0x9000000000000000ULL;

    board->piece_table[59] = KING;
    board->piece_table[60] = PIECE_NONE;
    board->piece_table[61] = PIECE_NONE;
    board->piece_table[63] = ROOK;
  }
}

// TODO: check capture flag instead of piece board
void board_make_move(Board *board, Move move) {
  uint8_t from_square = move_from_square(move);
  uint8_t to_square = move_to_square(move);
  uint8_t flags = move_flags(move);

  GameStateDetails new_state = {0}; // TODO: push and then edit to save copy
  new_state.halfmove_clock = BOARD_CURR_STATE(board).halfmove_clock + 1;
  new_state.castling_rights = BOARD_CURR_STATE(board).castling_rights;
  new_state.captured_piece = PIECE_NONE;
  new_state.hash = toggle_turn(BOARD_CURR_STATE(board).hash);

  switch (flags) {
  case FLAGS_PAWN_PUSH:
    move_piece(board, &new_state, from_square, to_square, flags);
    new_state.halfmove_clock = 0;
    break;
  case FLAGS_DOUBLE_PUSH:
    move_piece(board, &new_state, from_square, to_square, flags);
    new_state.halfmove_clock = 0;
    int ep_shift =
        (board->to_move == WHITE) ? (to_square + 8) : (to_square - 8);
    new_state.en_passant = 1ULL << ep_shift;
    new_state.hash = toggle_en_passant(new_state.hash, new_state.en_passant);
    break;
  case FLAGS_SHORT_CASTLE:
    short_castle(board, &new_state);
    break;
  case FLAGS_LONG_CASTLE:
    short_castle(board, &new_state);
    break;
  default:
    move_piece(board, &new_state, from_square, to_square, flags);
    // clang-format off
    if (flags & FLAGS_KNIGHT_PROMOTION) { set_piece(board, &new_state, to_square, KNIGHT); }
    if (flags & FLAGS_BISHOP_PROMOTION) { set_piece(board, &new_state, to_square, BISHOP); }
    if (flags & FLAGS_ROOK_PROMOTION) { set_piece(board, &new_state, to_square, ROOK); }
    if (flags & FLAGS_QUEEN_PROMOTION) { set_piece(board, &new_state, to_square, QUEEN); }
    // clang-format on
    break;
  }
  GameStateDetailsStack_push(&board->game_state_stack, new_state);
  board->to_move = OPPOSITE_COLOR(board->to_move);
}

void board_unmake_move(Board *board, Move move) {
  Piece captured_piece =
      GameStateDetailsStack_pop(&board->game_state_stack).captured_piece;
  uint8_t from_square = move_from_square(move);
  uint8_t to_square = move_to_square(move);
  uint8_t flags = move_flags(move);

  switch (flags) {
  case FLAGS_SHORT_CASTLE:
    reverse_short_castle(board);
    break;
  case FLAGS_LONG_CASTLE:
    reverse_long_castle(board);
    break;
  default:
    undo_move_piece(board, to_square, from_square, flags);
    if (captured_piece != PIECE_NONE) {
      ToMove color = OPPOSITE_COLOR(board->to_move);
      set_piece_no_hash(board, to_square, captured_piece);
    }
    break;
  }
  board->to_move = OPPOSITE_COLOR(board->to_move);
}

bool board_valid(Board *board) {
  uint64_t all_pieces = 0;
  for (int color = 0; color < 2; color++) {
    for (int piece = 0; piece < 6; piece++) {
      if (all_pieces & board->pieces[color][piece]) {
        return false;
      }
      all_pieces |= board->pieces[color][piece];
    }
  }
  if (all_pieces != board->occupied) {
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

char *square_to_string(int index) {
  char *out = malloc(3);
  out[0] = 'a' + (index % 8);
  out[1] = '1' + (index / 8);
  out[2] = '\0';
  return out;
}

char piece_char_at(Board *board, int rank, int file) {
  uint64_t square_bit = 1ULL << (rank * 8 + file);

  // clang-format off
  if (board->pieces[0][0] & square_bit) return 'P';
  if (board->pieces[0][1] & square_bit) return 'N';
  if (board->pieces[0][2] & square_bit) return 'B';
  if (board->pieces[0][3] & square_bit) return 'R';
  if (board->pieces[0][4] & square_bit) return 'Q';
  if (board->pieces[0][5] & square_bit) return 'K';
  if (board->pieces[1][0] & square_bit) return 'p';
  if (board->pieces[1][1] & square_bit) return 'n';
  if (board->pieces[1][2] & square_bit) return 'b';
  if (board->pieces[1][3] & square_bit) return 'r';
  if (board->pieces[1][4] & square_bit) return 'q';
  if (board->pieces[1][5] & square_bit) return 'k';
  // clang-format on
  return ' ';
}

#define BOARD_SIDE "+---+---+---+---+---+---+---+---+"
#define FILE_LABELS "  a   b   c   d   e   f   g   h  "

// Board output format stolen from stockfish (mine was ugly)
char *board_to_string(Board *board) {
  if (!board_valid(board)) {
    return "Invalid Board";
  }
  size_t buffer_size = 750;
  char *buffer = malloc(buffer_size);
  char *buffer_ptr = buffer;

  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                         "%s\n", BOARD_SIDE);

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 7; file >= 0; file--) {
      buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                             "| %c ", piece_char_at(board, rank, file));
    }

    buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                           "| %d\n%s\n", rank + 1, BOARD_SIDE);
  }
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                         "%s\n", FILE_LABELS);

  char *board_fen = board_to_fen(board);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                         "\nFen: %s", board_fen);
  free(board_fen);
  *buffer_ptr = '\0';

  return buffer;
}

char *board_to_debug_string(Board *board) {
  size_t buffer_size = 10000; // TODO: downsize a lot
  char *buffer = malloc(buffer_size);
  char *buffer_ptr = buffer;

  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                         "White Pawn: %lx\n", board->pieces[WHITE][PAWN]);
  // clang-format off
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "White Knight: %lx\n", board->pieces[WHITE][KNIGHT]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "White Bishop: %lx\n", board->pieces[WHITE][BISHOP]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "White Rook: %lx\n", board->pieces[WHITE][ROOK]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "White Queen: %lx\n", board->pieces[WHITE][QUEEN]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "White King: %lx\n", board->pieces[WHITE][KING]);

  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "Black Pawn: %lx\n", board->pieces[BLACK][PAWN]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "Black Knight: %lx\n", board->pieces[BLACK][KNIGHT]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "Black Bishop: %lx\n", board->pieces[BLACK][BISHOP]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "Black Rook: %lx\n", board->pieces[BLACK][ROOK]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "Black Queen: %lx\n", board->pieces[BLACK][QUEEN]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "Black King: %lx\n", board->pieces[BLACK][KING]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "\n");

  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "White Occupied: %lx\n", board->occupied_by_color[WHITE]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "Black Occupied: %lx\n", board->occupied_by_color[BLACK]);
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "Occupied: %lx\n", board->occupied);

  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                         "To Move: %s\n", board->to_move == WHITE ? "White"
                                                                  : "Black");
  
  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "\n");
  // clang-format on

  buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                         "Piece Table:\n");
  for (int i = 63; i >= 0; i--) {
    buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                           "%d ", board->piece_table[i]);
    if (i % 8 == 0) {
      buffer_ptr +=
          snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer), "\n");
    }
  }

  for (int i = 0; i <= board->game_state_stack.top; i++) {
    GameStateDetails state =
        GameStateDetailsStack_peek_down(&board->game_state_stack, i);
    buffer_ptr += snprintf(buffer_ptr, buffer_size - (buffer_ptr - buffer),
                           "State "
                           "%d:\nhalfmove_clock: %d\ncastling_rights: %d\nen_"
                           "passant: %lx\ncaptured_piece: %d\nhash: %lx\n",
                           i, state.halfmove_clock, state.castling_rights,
                           state.en_passant, state.captured_piece, state.hash);
  }
  *buffer_ptr = '\0';
  return buffer;
}
