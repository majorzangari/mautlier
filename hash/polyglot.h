#ifndef MAUTLIER_POLYGLOT_H
#define MAUTLIER_POLYGLOT_H

#include "move.h"
#include <stdint.h>

typedef struct {
  uint64_t key;
  uint16_t move;
  uint16_t weight;
  uint32_t learn;
} PolyglotEntry;

Move lookup_book_move(uint64_t key, FILE *book, Board *pos);

#endif
