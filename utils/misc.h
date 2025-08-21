#ifndef MAUTLIER_MISC_UTIL_H
#define MAUTLIER_MISC_UTIL_H

#include "board.h"
#include <stdarg.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

bool or_strcmp(const char *str, int count, ...);

// returns a malloced array of strings, last element is NULL
// should free the array with split_free
char **split_whitespace(char *str);

void split_free(char **str_array);

void order_alphabetically(Move *moves, int num_moves);

#endif
