//
// Created by Major Zangari on 4/19/25.
//

#include "fen.h"
#include "bithelpers.h"
#include "debug_printer.h"
#include "diagnostic_tools.h"
#include "hash.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "move.h" // TODO yucky yucky move these functions somewhere else
#include "stdio.h"

void populate_pieces(uint64_t pieces[2][6], char *pieces_data) {
  int rank = 7;
  int file = 7;

  for (char *c = pieces_data; *c != '\0' && rank >= 0; c++) {
    char piece = *c;
    if (piece == '/') {
      rank--;
      file = 7;
    } else if (piece >= '1' && piece <= '8') {
      int empty_squares = piece - '0';
      file -= empty_squares;
    } else {
      int index = rank * 8 + file;
      uint64_t square_bit = 1ULL << index;
      // clang-format off

      switch (piece) {
        case 'P': pieces[WHITE][0] |= square_bit; break;
        case 'N': pieces[WHITE][1] |= square_bit; break;
        case 'B': pieces[WHITE][2] |= square_bit; break;
        case 'R': pieces[WHITE][3] |= square_bit; break;
        case 'Q': pieces[WHITE][4] |= square_bit; break;
        case 'K': pieces[WHITE][5] |= square_bit; break;

        case 'p': pieces[BLACK][0] |= square_bit; break;
        case 'n': pieces[BLACK][1] |= square_bit; break;
        case 'b': pieces[BLACK][2] |= square_bit; break;
        case 'r': pieces[BLACK][3] |= square_bit; break;
        case 'q': pieces[BLACK][4] |= square_bit; break;
        case 'k': pieces[BLACK][5] |= square_bit; break;
      }
      // clang-format on
      file--;
    }
  }
}

Bitboard square_to_bit(const char *square) {
  if (strlen(square) != 2) {
    return 0;
  }

  Bitboard out = 1;

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

Board *fen_to_board(char *input_fen) {
  DP_PRINTF("FUNC_TRACE", "fen_to_board\n");
  char fen[100];
  strcpy(fen, input_fen); // slow ig but helps outside a lot

  Board *out = calloc(1, sizeof(Board));
  out->game_state = GS_ONGOING;

  char *saveptr;

  char *pieces_data = strtok_r(fen, " ", &saveptr);
  populate_pieces(out->pieces, pieces_data);
  board_update_occupied(out);
  board_update_piece_table(out);

  char *to_move_data = strtok_r(NULL, " ", &saveptr);

  if (*to_move_data == 'w') {
    out->to_move = WHITE_TO_MOVE;
  } else if (*to_move_data == 'b') {
    out->to_move = BLACK_TO_MOVE;
  } else {
    free(out);
    return NULL;
  }
  GameStateDetails details = {0};
  char *castling_rights_data = strtok_r(NULL, " ", &saveptr);
  details.castling_rights = parse_castling_rights(castling_rights_data);

  char *en_passant_data = strtok_r(NULL, " ", &saveptr);
  int en_passant = square_to_bit(en_passant_data);

  int ep_file = en_passant % 8;
  int ep_shift = lsb_index(en_passant);

  if (out->to_move == BLACK) {
    if ((ep_file > 0 && out->pieces[BLACK][PAWN] & (1ULL << (ep_shift + 7))) ||
        (ep_file < 7 && out->pieces[BLACK][PAWN] & (1ULL << (ep_shift + 9)))) {
      details.en_passant = 1ULL << ep_shift;
    }
  } else {
    if ((ep_file > 0 && out->pieces[WHITE][PAWN] & (1ULL << (ep_shift - 9))) ||
        (ep_file < 7 && out->pieces[WHITE][PAWN] & (1ULL << (ep_shift - 7)))) {
      details.en_passant = 1ULL << ep_shift;
    }
  }

  char *halfmove_clock_data = strtok_r(NULL, " ", &saveptr);
  details.halfmove_clock = atoi(halfmove_clock_data);

  details.captured_piece = PIECE_NONE;

  GameStateDetailsStack_init(&out->game_state_stack);
  GameStateDetailsStack_push(&out->game_state_stack, details);
  BOARD_CURR_STATE(out).hash = zobrist_hash(out); // feels hacky
  return out;
}

char *board_to_fen(Board *board) {
  DP_PRINTF("FUNC_TRACE", "board_to_fen\n");
  size_t buffer_size = 95;
  char *out = malloc(buffer_size);
  int index = 0;

  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 7; file >= 0; file--) {
      int square_index = rank * 8 + file;
      Bitboard square_bit = 1ULL << square_index;
      // clang-format off
      if      ((board->pieces[WHITE][PAWN] & square_bit) > 0) out[index++] = 'P';
      else if ((board->pieces[WHITE][KNIGHT] & square_bit) > 0) out[index++] = 'N';
      else if ((board->pieces[WHITE][BISHOP] & square_bit) > 0) out[index++] = 'B';
      else if ((board->pieces[WHITE][ROOK] & square_bit) > 0) out[index++] = 'R';
      else if ((board->pieces[WHITE][QUEEN] & square_bit) > 0) out[index++] = 'Q';
      else if ((board->pieces[WHITE][KING] & square_bit) > 0) out[index++] = 'K';

      else if ((board->pieces[BLACK][PAWN] & square_bit) > 0) out[index++] = 'p';
      else if ((board->pieces[BLACK][KNIGHT] & square_bit) > 0) out[index++] = 'n';
      else if ((board->pieces[BLACK][BISHOP] & square_bit) > 0) out[index++] = 'b';
      else if ((board->pieces[BLACK][ROOK] & square_bit) > 0) out[index++] = 'r';
      else if ((board->pieces[BLACK][QUEEN] & square_bit) > 0) out[index++] = 'q';
      else if ((board->pieces[BLACK][KING] & square_bit) > 0) out[index++] = 'k';
      // clang-format on
      else {
        int empty_squares = 0;
        while (file >= 0 && ((board->occupied & square_bit) == 0)) {
          empty_squares++;
          square_bit >>= 1;
          file--;
        }
        index +=
            snprintf(out + index, buffer_size - index, "%d", empty_squares);
        file++;
      }
    }
    if (rank != 0)
      out[index++] = '/';
  }

  out[index++] = ' ';
  out[index++] = (board->to_move == WHITE_TO_MOVE) ? 'w' : 'b';
  out[index++] = ' ';

  GameStateDetails curr_state = BOARD_CURR_STATE(board);

  // clang-format off
  if ((curr_state.castling_rights | CR_WHITE_SHORT) > 0) out[index++] = 'K';
  if ((curr_state.castling_rights | CR_BLACK_SHORT) > 0) out[index++] = 'k';
  if ((curr_state.castling_rights | CR_WHITE_LONG) > 0) out[index++] = 'Q';
  if ((curr_state.castling_rights | CR_BLACK_LONG) > 0) out[index++] = 'q';
  // clang-format on

  out[index++] = ' ';
  if (curr_state.en_passant == 0)
    out[index++] = '-';
  else {
    int ep_index = lsb_index(curr_state.en_passant);
    char *sq = square_to_string(ep_index);
    strcpy(out + index, sq);
    index += 2;
  }

  index +=
      snprintf(out + index, buffer_size - index, " %d %d",
               curr_state.halfmove_clock, board->game_state_stack.top / 2 + 1);
  out[index] = '\0';
  return out;
}
