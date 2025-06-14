//
// Created by Major Zangari on 4/19/25.
//

#include "board.h"
#include "fen.h"
#include "move.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

Board *init_default_board() {
  return fen_to_board(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

static inline void move_piece(Board *board, int from_square, int to_square) {
  int piece = board->piece_table[from_square];
  int color = board->to_move;
  board->pieces[color][piece] &= ~(1ULL << from_square);
  board->pieces[color][piece] |= (1ULL << to_square);
  if (board->piece_table[to_square] != PIECE_NONE) {
    int captured_color = color == WHITE ? BLACK : WHITE;
    int captured_piece = board->piece_table[to_square];
    board->pieces[captured_color][captured_piece] &= ~(1ULL << to_square);
  }
  board->piece_table[to_square] = piece;
  board->piece_table[from_square] = PIECE_NONE;
}

static inline void set_piece(Board *board, int square, Piece piece) {
  int color = board->to_move;
  int previous_piece = board->piece_table[square];
  if (previous_piece != PIECE_NONE) {
    board->pieces[color][previous_piece] &= ~(1ULL << square);
  }
  board->pieces[color][piece] |= (1ULL << square);
  board->piece_table[square] = piece;
}

void board_make_move(Board *board, Move move) {
  uint8_t from_square = move_from_square(move);
  uint8_t to_square = move_to_square(move);
  uint8_t flags = move_flags(move);

  board->en_passant = 0;
  switch (flags) {
  case FLAGS_DOUBLE_PUSH:
    move_piece(board, from_square, to_square);
    board->en_passant = (board->to_move == WHITE) ? (1ULL << (from_square + 8))
                                                  : (1ULL << (from_square - 8));
    break;
  case FLAGS_SHORT_CASTLE:
    if (board->to_move == WHITE) {
      board->castling_rights &= ~(CR_WHITE_SHORT | CR_WHITE_LONG);
      board->pieces[WHITE][KING] |= 0xAULL;
      board->pieces[WHITE][ROOK] |= 0x5ULL;
      board->piece_table[0] = PIECE_NONE;
      board->piece_table[1] = KING;
      board->piece_table[2] = ROOK;
      board->piece_table[3] = PIECE_NONE;
    } else {
      board->castling_rights &= ~(CR_BLACK_SHORT | CR_BLACK_LONG);
      board->pieces[BLACK][KING] |= 0xA00000000000000ULL;
      board->pieces[BLACK][ROOK] |= 0x500000000000000ULL;
      board->piece_table[56] = PIECE_NONE;
      board->piece_table[57] = KING;
      board->piece_table[58] = ROOK;
      board->piece_table[59] = PIECE_NONE;
    }
    break;
  case FLAGS_LONG_CASTLE:
    if (board->to_move == WHITE) {
      board->castling_rights &= ~(CR_WHITE_SHORT | CR_WHITE_LONG);
      board->pieces[WHITE][KING] |= 0x28ULL;
      board->pieces[WHITE][ROOK] |= 0x90ULL;
      board->piece_table[3] = PIECE_NONE;
      board->piece_table[4] = ROOK;
      board->piece_table[5] = KING;
      board->piece_table[7] = PIECE_NONE;
    } else {
      board->castling_rights &= ~(CR_BLACK_SHORT | CR_BLACK_LONG);
      board->pieces[BLACK][KING] |= 0x2800000000000000ULL;
      board->pieces[BLACK][ROOK] |= 0x9000000000000000ULL;
      board->piece_table[59] = PIECE_NONE;
      board->piece_table[60] = ROOK;
      board->piece_table[61] = KING;
      board->piece_table[63] = PIECE_NONE;
    }
    break;
  default:
    move_piece(board, from_square, to_square);
    // clang-format off
    if (flags & FLAGS_KNIGHT_PROMOTION) set_piece(board, to_square, KNIGHT);
    if (flags & FLAGS_BISHOP_PROMOTION) set_piece(board, to_square, BISHOP);
    if (flags & FLAGS_BISHOP_PROMOTION) set_piece(board, to_square, ROOK);
    if (flags & FLAGS_QUEEN_PROMOTION) set_piece(board, to_square, QUEEN);
  }
  board->to_move = 1 - board->to_move; // Switch turns
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
