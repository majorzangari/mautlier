#ifndef MAUTLIER_DIAGNOSTIC_TOOLS_H
#define MAUTLIER_DIAGNOSTIC_TOOLS_H

#include "board.h"

char *square_to_string(int index);
char *board_to_string(Board *board);

char *board_to_debug_string(Board *board);

bool compare_boards(Board *board1, Board *board2);
char *last_board_compare_diff();

#endif
