#ifndef MAUTLIER_PERFT_H
#define MAUTLIER_PERFT_H

#include "board.h"
#include "move.h"

#include <stdint.h>

uint64_t perft(Board *board, int depth);

void perft_divide(Board *board, int depth);

uint64_t safe_perft(Board *board, int depth);

#endif
