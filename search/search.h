#ifndef MAUTLIER_SEARCH_H
#define MAUTLIER_SEARCH_H

#include "move.h"

// inf might be a misnomer, just a big number to trump other factors
#define INF_SCORE 0xFFFFFF

#define MATE_SCORE 100000

#define MAX_PLY 64 // doubt im ever even hitting this

typedef struct {
  int max_depth; // max depth to search, or 0 for no max depth
  long max_duration_ms;
  long max_nodes; // max nodes to search, or 0 for no max nodes
  int infinite;
} SearchRequestInfo;

void search_position(Board *board, SearchRequestInfo *info, FILE *book);

#endif
