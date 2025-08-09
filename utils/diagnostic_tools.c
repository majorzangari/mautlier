#include "diagnostic_tools.h"
#include "fen.h"

#include <stdarg.h>
#include <stdlib.h>

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
    printf("%s\n", board_to_debug_string(board));
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

#define DIFF_BUFFER_SIZE 1000 // TODO: downsize a lot
static char last_compare_diff[DIFF_BUFFER_SIZE];

void set_errorf(char *msg, ...) {
  va_list args;
  va_start(args, msg);
  vsnprintf(last_compare_diff, DIFF_BUFFER_SIZE, msg, args);
  va_end(args);
}

bool compare_boards(Board *board1, Board *board2) {

  // Piece bitboards
  for (ToMove color = WHITE; color <= BLACK; color++) {
    for (Piece piece = PAWN; piece <= KING; piece++) {
      if (board1->pieces[color][piece] != board2->pieces[color][piece]) {
        set_errorf("Piece bitboards differ for color %d piece %d: %lx vs %lx",
                   color, piece, board1->pieces[color][piece],
                   board2->pieces[color][piece]);
        return false;
      }
    }
  }

  // Occupied bitboards
  if (board1->occupied_by_color[WHITE] != board2->occupied_by_color[WHITE]) {
    set_errorf("Occupied by white differ: %lx vs %lx",
               board1->occupied_by_color[WHITE],
               board2->occupied_by_color[WHITE]);
    return false;
  }
  if (board1->occupied_by_color[BLACK] != board2->occupied_by_color[BLACK]) {
    set_errorf("Occupied by black differ: %lx vs %lx",
               board1->occupied_by_color[BLACK],
               board2->occupied_by_color[BLACK]);
    return false;
  }

  if (board1->occupied != board2->occupied) {
    set_errorf("Occupied differ: %lx vs %lx", board1->occupied,
               board2->occupied);
    return false;
  }

  // To move
  if (board1->to_move != board2->to_move) {
    set_errorf("To move differ: %d vs %d", board1->to_move, board2->to_move);
    return false;
  }

  // Piece tables
  for (int i = 63; i >= 0; i--) {
    if (board1->piece_table[i] != board2->piece_table[i]) {
      char comp_string[512];
      int index = 0;
      for (int j = 7; j >= 0; j--) {
        for (int k = 7; k >= 0; k--) {
          comp_string[index++] = board1->piece_table[j * 8 + k];
          comp_string[index++] = ' ';
        }
        comp_string[index++] = '\t';
        for (int k = 7; k >= 0; k--) {
          comp_string[index++] = board2->piece_table[j * 8 + k];
          comp_string[index++] = ' ';
        }
        comp_string[index++] = '\n';
      }
      comp_string[index] = '\0';

      set_errorf("Piece table differ at index %d:\n%s", i, comp_string);
      return false;
    }
  }

  // Game state stacks
  for (int i = 0; i <= board1->game_state_stack.top; i++) {
    if (i > board2->game_state_stack.top) {
      set_errorf("Game state stack size differ: %d vs %d",
                 board1->game_state_stack.top + 1,
                 board2->game_state_stack.top + 1);
      return false;
    }
    GameStateDetails state1 =
        GameStateDetailsStack_peek_down(&board1->game_state_stack, i);
    GameStateDetails state2 =
        GameStateDetailsStack_peek_down(&board2->game_state_stack, i);
    if (state1.halfmove_clock != state2.halfmove_clock) {
      set_errorf("Halfmove clock differ at state %d: %d vs %d", i,
                 state1.halfmove_clock, state2.halfmove_clock);
      return false;
    }
    if (state1.castling_rights != state2.castling_rights) {
      set_errorf("Castling rights differ at state %d: %d vs %d", i,
                 state1.castling_rights, state2.castling_rights);
      return false;
    }
    if (state1.en_passant != state2.en_passant) {
      set_errorf("En passant differ at state %d: %lx vs %lx", i,
                 state1.en_passant, state2.en_passant);
      return false;
    }
    if (state1.captured_piece != state2.captured_piece) {
      set_errorf("Captured piece differ at state %d: %d vs %d", i,
                 state1.captured_piece, state2.captured_piece);
      return false;
    }
    if (state1.hash != state2.hash) {
      set_errorf("Hash differ at state %d: %lx vs %lx", i, state1.hash,
                 state2.hash);
      return false;
    }
  }

  set_errorf("No differences found");
  return true;
}

char *last_board_compare_diff() { return last_compare_diff; }
