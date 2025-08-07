//
// Created by Major Zangari on 4/19/25.
//

#ifndef MAUTLIER_BOARD_H
#define MAUTLIER_BOARD_H

#include <stdbool.h>
#include <stdint.h>

#include "stack.h"

// from move.h
typedef uint16_t Move;

#define WHITE 0
#define BLACK 1

#define OPPOSITE_COLOR(color) ((color) == WHITE ? BLACK : WHITE)

typedef uint8_t Piece;

#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5
#define PIECE_NONE 6

#define CR_NONE 0
#define CR_WHITE_SHORT 1
#define CR_WHITE_LONG 2
#define CR_BLACK_SHORT 4
#define CR_BLACK_LONG 8

#define RANK_1 0x00000000000000FFULL
#define RANK_2 0x000000000000FF00ULL
#define RANK_3 0x0000000000FF0000ULL
#define RANK_4 0x00000000FF000000ULL
#define RANK_5 0x000000FF00000000ULL
#define RANK_6 0x0000FF0000000000ULL
#define RANK_7 0x00FF000000000000ULL
#define RANK_8 0xFF00000000000000ULL

#define FILE_A 0x8080808080808080ULL
#define FILE_B 0x4040404040404040ULL
#define FILE_C 0x2020202020202020ULL
#define FILE_D 0x1010101010101010ULL
#define FILE_E 0x0808080808080808ULL
#define FILE_F 0x0404040404040404ULL
#define FILE_G 0x0202020202020202ULL
#define FILE_H 0x0101010101010101ULL

#define WHITE_SHORT_CASTLE_CLEARANCE_MASK 0x06ULL
#define WHITE_LONG_CASTLE_CLEARANCE_MASK 0x70ULL
#define BLACK_SHORT_CASTLE_CLEARANCE_MASK 0x0600000000000000ULL
#define BLACK_LONG_CASTLE_CLEARANCE_MASK 0x7000000000000000ULL

typedef enum {
  WHITE_TO_MOVE = 0,
  BLACK_TO_MOVE = 1,
} ToMove;

// TODO: color and ToMove are different? bad design gotta fix

#define OPPOSITE_TO_MOVE(color) ((color) == WHITE_TO_MOVE ? BLACK_TO_MOVE : WHITE_TO_MOVE

typedef enum { // TODO: could maybe improve performance by leaving checkmates
               // unknown until checking moves next
  GS_ONGOING,
  GS_WHITE_WON,
  GS_BLACK_WON,
  GS_DRAW_STALEMATE,
  GS_DRAW_BY_REPETITION,
  GS_DRAW_FIFTY_MOVE_RULE,
  GS_DRAW_INSUFFICIENT_MATERIAL,
  GS_INVALID,
} GameState;

typedef uint64_t Bitboard;

// contains all gamestate details that may be lost between moves
typedef struct GameStateDetails {
  uint8_t halfmove_clock;
  uint8_t castling_rights;
  Bitboard en_passant;
  Piece captured_piece; // PIECE_NONE if no piece was captured on that turn (or
                        // current state)
  uint64_t hash;
} GameStateDetails;

#define MAX_SAVED_GAMESTATES                                                   \
  16384 // Max a chess game could need is ~12,000 but 99% will be less than 512
        // (538 was the most any tournament game took)
DEFINE_STACK_TYPE(GameStateDetailsStack, GameStateDetails, MAX_SAVED_GAMESTATES)

typedef struct Board {
  Bitboard pieces[2][6];
  Bitboard occupied_by_color[2];
  Bitboard occupied;
  ToMove to_move;
  Piece piece_table[64];
  GameStateDetailsStack game_state_stack;
} Board;

#define BOARD_CURR_STATE(board)                                                \
  ((board)->game_state_stack.data[(board)->game_state_stack.top])

Board *init_default_board();
void board_make_move(Board *board, Move move);
void board_unmake_move(Board *board, Move move);

// slow, should only be run when setting up board
void board_update_occupied(Board *board);
// slow, should only be run when setting up board
void board_update_piece_table(Board *board);

bool board_white_check(Board *board);
bool board_black_check(Board *board);

// checks if board represents a valid state TODO: finish checking everything
bool board_valid(Board *board);

#endif // BOARD_H
