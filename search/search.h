#ifndef MAUTLIER_SEARCH_H
#define MAUTLIER_SEARCH_H

#include "move.h"

// inf might be a misnomer, just a big number to trump other factors
#define INF_SCORE 0xFFFFFFF
#define NEG_INF_SCORE -0xFFFFFFF

typedef struct {
  int depth;
  long startTime;
  long endTime;
  long nodes;
  int infinite;
  int stop;
  int stopped;
} SearchInfo;

void search_position(Board *board, SearchInfo *info);

#endif
