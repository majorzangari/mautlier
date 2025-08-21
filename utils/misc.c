#include "misc.h"
#include "board.h"
#include "move.h"

#include <stdlib.h>
#include <string.h>

bool or_strcmp(const char *str, int count, ...) {
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

char **split_whitespace(char *str) {
  if (!str) {
    return NULL;
  }
  char *copy = strdup(str);
  if (!copy) {
    return NULL;
  }

  size_t size = 10;
  size_t token_count = 0;
  char **out = malloc(size * sizeof(char *));

  const char *delimiters = " \t\r\n\v";
  char *saveptr;
  char *token = strtok_r(copy, delimiters, &saveptr);

  while (token) {
    if (token_count + 1 >= size) {
      size *= 2;
      out = realloc(out, size * sizeof(char *));
    }
    out[token_count++] = strdup(token); // TODO: i might not have to strdup
                                        // here, prob doesn't matter tho
    token = strtok_r(NULL, delimiters, &saveptr);
  }

  out[token_count] = NULL;
  free(copy);
  return out;
}

void split_free(char **str_array) {
  size_t index = 0;
  while (str_array[index] != NULL) {
    free(str_array[index]);
    index++;
  }
  free(str_array);
}
