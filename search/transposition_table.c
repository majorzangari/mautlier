#include "transposition_table.h"

#include "search.h"
#include <stdlib.h>

TTEntry lt_table[TT_SIZE];

#define MATE_THRESHOLD INF_SCORE - MAX_PLY

int get_adjusted_score(TTEntry *entry, int curr_ply) {
  int score = entry->score;
  int stored_ply = entry->ply;
  if (abs(score) > MATE_THRESHOLD) {
    return score + (score > 0 ? stored_ply - curr_ply : curr_ply - stored_ply);
  }
  return score;
}

TTEntry *tt_query(uint64_t key) {
  size_t index = key % TT_SIZE;
  TTEntry *entry = &lt_table[index];
  if (entry->key == key) {
    return entry;
  }
  return NULL;
}

void tt_store(uint64_t key, int depth, int score, int orig_alpha, int beta,
              int ply, Move best_move) {
  size_t index = key & (TT_SIZE - 1);
  TTEntry *entry =
      &lt_table[TT_SIZE]; // TODO: figure out if this should be a straight array
                          // lookup. Could promote saving deeper searches?

  TTEntryType type;
  if (score >= beta) {
    type = TT_LOWERBOUND;
    score = beta;
  } else if (score <= orig_alpha) {
    type = TT_UPPERBOUND;
    score = orig_alpha;
  } else {
    type = TT_EXACT;
  }

  if (!entry || entry->depth < depth) { // Replace only if deeper or equal
    entry->key = key;
    entry->depth = depth;
    entry->score = score;
    entry->type = type;
    entry->best_move = best_move;
    entry->ply = ply;
  }
}
