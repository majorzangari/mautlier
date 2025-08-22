#ifndef MAUTLIER_TRANSPOSITION_TABLE
#define MAUTLIER_TRANSPOSITION_TABLE

#include "move.h"
#include <stdint.h>

#define TT_SIZE (1 << 22) // 4M entries, ~160MB?

typedef enum { TT_EXACT, TT_LOWERBOUND, TT_UPPERBOUND } TTEntryType;

typedef struct {
  uint64_t key;
  int depth;
  int score;
  TTEntryType type;
  Move best_move;
} TTEntry;

void tt_store(uint64_t key, int depth, int score, TTEntryType type,
              Move best_move);

TTEntry *tt_query(uint64_t key);

#endif
