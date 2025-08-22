#include "transposition_table.h"

TTEntry lt_table[TT_SIZE];

void tt_store(uint64_t key, int depth, int score, TTEntryType type,
              Move best_move) {
  size_t index = key % TT_SIZE;
  TTEntry *entry = &lt_table[index];
  if (entry->depth <= depth) { // Replace only if deeper or equal
    entry->key = key;
    entry->depth = depth;
    entry->score = score;
    entry->type = type;
    entry->best_move = best_move;
  }
}

TTEntry *tt_query(uint64_t key) {
  size_t index = key % TT_SIZE;
  TTEntry *entry = &lt_table[index];
  if (entry->key == key) {
    return entry;
  }
  return NULL;
}
