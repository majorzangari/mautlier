#include "polyglot.h"
#include "move.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PolyglotEntry read_polyglot_entry(FILE *file) {
  PolyglotEntry e;

  fread(&e, sizeof(PolyglotEntry), 1, file);

  e.key = __builtin_bswap64(e.key);
  e.move = __builtin_bswap16(e.move);
  e.weight = __builtin_bswap16(e.weight);
  e.learn = __builtin_bswap32(e.learn);

  return e;
}

#define PROMO_PIECES " nbrq"

static inline void square_to_algebraic(int sq, char *out) {
  int file = sq % 8;
  int rank = sq / 8;
  out[0] = 'a' + file;
  out[1] = '1' + rank;
  out[2] = '\0';
}

static inline Move decode_polyglot_move(PolyglotEntry *entry, Board *pos) {
  uint16_t move_poly = entry->move;

  int from = (move_poly >> 6) & 0x3F;
  int to = move_poly & 0x3F;
  int promo = (move_poly >> 12) & 0x7;

  char from_str[3];
  char to_str[3];
  square_to_algebraic(from, from_str);
  square_to_algebraic(to, to_str);

  char algebraic[6];

  sprintf(algebraic, "%s%s", from_str, to_str);

  if (promo > 0 && promo < 5) {
    size_t len = strlen(algebraic);
    algebraic[len] = PROMO_PIECES[promo];
    algebraic[len + 1] = '\0';
  }

  return algebraic_to_move(pos, algebraic);
}

static inline Move pick_move(PolyglotEntry *moves, int count, Board *pos) {
  int total_weight = 0;

  for (int i = 0; i < count; i++) {
    total_weight += moves[i].weight;
  }

  int rand_val = rand() % total_weight;
  int acc_weight = 0;

  for (int i = 0; i < count; i++) {
    acc_weight += moves[i].weight;
    if (rand_val < acc_weight) {
      return decode_polyglot_move(&moves[i], pos);
    }
  }

  return NULL_MOVE;
}

#define MAX_BOOK_MOVES_CONSIDER 10

Move lookup_book_move(uint64_t key, FILE *book, Board *pos) {
  fseek(book, 0, SEEK_END);
  size_t book_size = ftell(book);
  fseek(book, 0, SEEK_SET);

  int left = 0;
  int right = book_size / sizeof(PolyglotEntry) - 1;

  int count = 0;

  PolyglotEntry entries[MAX_BOOK_MOVES_CONSIDER];

  while (left <= right) {
    int mid = (left + right) / 2;
    fseek(book, mid * sizeof(PolyglotEntry), SEEK_SET);

    PolyglotEntry entry = read_polyglot_entry(book);

    if (entry.key == key) {
      int left = mid;
      while (left > 0) {
        fseek(book, (left - 1) * sizeof(PolyglotEntry), SEEK_SET);
        PolyglotEntry left_entry = read_polyglot_entry(book);
        if (left_entry.key != key) {
          break;
        }
        left--;
      }

      while (left <= right && count < MAX_BOOK_MOVES_CONSIDER) {
        fseek(book, left * sizeof(PolyglotEntry), SEEK_SET);
        PolyglotEntry entry = read_polyglot_entry(book);
        if (entry.key != key) {
          break;
        }
        entries[count++] = entry;
        left++;
      }

      return pick_move(entries, count, pos);
    } else if (entry.key < key) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }

  return NULL_MOVE;
}
