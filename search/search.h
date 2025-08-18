#ifndef MAUTLIER_SEARCH_H
#define MAUTLIER_SEARCH_H

#include "move.h"

// inf might be a misnomer, just a big number to trump other factors
#define INF_SCORE 0xFFFFFF
#define NEG_INF_SCORE -0xFFFFFF

Move lazy_search(Board *board, int depth);

#endif
