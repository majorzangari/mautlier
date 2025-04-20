//
// Created by Major Zangari on 4/19/25.
//

#include "fen.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "stdio.h"

void populate_pieces(uint64_t pieces[2][6], char *pieces_data) {
  int rank = 7;
  int file = 0;

  for (char *c = pieces_data; *c != '\0' && rank >= 0; c++) {
    char piece = *c;
    if (piece == '/') {
      rank--;
      file = 0;
    } else if (piece >= '1' && piece <= '8') {
      int empty_squares = piece - '0';
      file += empty_squares;
    } else {
      int index = rank * 8 + file;
      uint64_t square_bit = 1ULL << index;
      // clang-format off

      printf("[%c, %d, %d, %ld]", piece, rank, file, square_bit);
      switch (piece) {
        case 'P': pieces[0][0] |= square_bit; break;
        case 'N': pieces[0][1] |= square_bit; break;
        case 'B': pieces[0][2] |= square_bit; break;
        case 'R': pieces[0][3] |= square_bit; break;
        case 'Q': pieces[0][4] |= square_bit; break;
        case 'K': pieces[0][5] |= square_bit; break;

        case 'p': pieces[1][0] |= square_bit; break;
        case 'n': pieces[1][1] |= square_bit; break;
        case 'b': pieces[1][2] |= square_bit; break;
        case 'r': pieces[1][3] |= square_bit; break;
        case 'q': pieces[1][4] |= square_bit; break;
        case 'k': pieces[1][5] |= square_bit; break;
        default: break;
      }
      // clang-format on
      file++;
    }
  }
}

uint64_t square_to_bit(const char *square) {
  if (strlen(square) != 2) {
    return 0;
  }

  uint64_t out = 1;

  // clang-format off
  switch (*square) {
    case 'a': out <<= 7; break;
    case 'b': out <<= 6; break;
    case 'c': out <<= 5; break;
    case 'd': out <<= 4; break;
    case 'e': out <<= 3; break;
    case 'f': out <<= 2; break;
    case 'g': out <<= 1; break;
    case 'h': out <<= 0; break;
    default: return 0;
  }
 
  switch (*++square) {
    case '1': out <<= (8 * 0); break;
    case '2': out <<= (8 * 1); break;
    case '3': out <<= (8 * 2); break;
    case '4': out <<= (8 * 3); break;
    case '5': out <<= (8 * 4); break;
    case '6': out <<= (8 * 5); break;
    case '7': out <<= (8 * 6); break;
    case '8': out <<= (8 * 7); break;
    default: return 0;
  }
  // clang-format on

  return out;
}

uint8_t parse_castling_rights(char *castling_rights_data) {
  uint8_t out = 0;
  // clang-format off
  for (char *c = castling_rights_data; *c != '\0'; c++) {
    switch (*c) {
      case 'K': out |= CR_WHITE_SHORT; break;
      case 'k': out |= CR_BLACK_SHORT; break;
      case 'Q': out |= CR_WHITE_LONG; break;
      case 'q': out |= CR_BLACK_LONG; break;
      default: break;
    }
  }
  // clang-format on
  return out;
}

Board *fen_to_board(char *fen) {
  Board *out = malloc(sizeof(Board));
  char *saveptr;

  char *pieces_data = strtok_r(fen, " ", &saveptr);
  populate_pieces(out->pieces, pieces_data);

  char *to_move_data = strtok_r(NULL, " ", &saveptr);

  if (*to_move_data == 'w') {
    out->to_move = WHITE_TO_MOVE;
  } else if (*to_move_data == 'b') {
    out->to_move = BLACK_TO_MOVE;
  } else {
    free(out);
    return NULL;
  }

  char *castling_rights_data = strtok_r(NULL, " ", &saveptr);
  out->castling_rights = parse_castling_rights(castling_rights_data);

  char *en_passant_data = strtok_r(NULL, " ", &saveptr);
  out->en_passant = square_to_bit(en_passant_data);

  char *halfmove_clock_data = strtok_r(NULL, " ", &saveptr);
  out->halfmove_clock = atoi(halfmove_clock_data);

  return out;
}
