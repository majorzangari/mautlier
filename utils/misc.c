#include "misc.h"
#include "board.h"
#include "move.h"

#include <string.h>

bool or_strcmp(char *str, int count, ...) {
  va_list args;
  va_start(args, count);

  for (int i = 0; i < count; i++) {
    const char *cmp_str = va_arg(args, const char *);
    if (strcmp(str, cmp_str) == 0) {
      va_end(args);
      return true;
    }
  }

  va_end(args);
  return false;
}

void order_alphabetically(Move *moves, int num_moves) {
  for (int i = 0; i < num_moves - 1; i++) {
    for (int j = i + 1; j < num_moves; j++) {
      char str1[16];
      memcpy(str1, move_to_string(moves[i]), sizeof(str1));
      char str2[16];
      memcpy(str2, move_to_string(moves[j]), sizeof(str2));

      if (strcmp(str1, str2) > 0) {
        Move temp = moves[i];
        moves[i] = moves[j];
        moves[j] = temp;
      }
    }
  }
}
