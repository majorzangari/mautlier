//
// Created by Major Zangari on 4/19/25.
//

#include "board.h"
#include "fen.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
