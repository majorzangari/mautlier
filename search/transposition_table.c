#include "transposition_table.h"

#include "eval.h"
#include "search.h"
#include <stdlib.h>

static TTEntry lt_table[TT_SIZE];

int get_adjusted_score(TTEntry *entry, int curr_ply) {
  int score = entry->score;

  if (score >= MATE_THRESHOLD) {
    return score + (curr_ply - entry->ply);
  } else if (score <= -MATE_THRESHOLD) {
    return score - (curr_ply - entry->ply);
  }

  // non-mate score, return as-is
  return score;
}

TTEntry *tt_query(uint64_t key) {
  // use same indexing as tt_store (power-of-two friendly)
  size_t index = key & (TT_SIZE - 1);
  TTEntry *entry = &lt_table[index];
  if (entry->key == key) {
    return entry;
  }
  return NULL;
}

void tt_store(uint64_t key, int depth, int score, int orig_alpha, int beta,
              int ply, Move best_move) {
  size_t index = key & (TT_SIZE - 1);
  TTEntry *entry = &lt_table[index];

  TTEntryType type;
  if (score >= beta) {
    type = TT_LOWERBOUND;
  } else if (score <= orig_alpha) {
    type = TT_UPPERBOUND;
  } else {
    type = TT_EXACT;
  }

  if (entry->key != key || depth >= entry->depth) {
    entry->key = key;
    entry->depth = depth;
    entry->score = score; // store actual score
    entry->type = type;
    entry->best_move = best_move;
    entry->ply = ply;
  }
}
