#ifndef MAUTLIER_MISC_UTIL_H
#define MAUTLIER_MISC_UTIL_H

#include "board.h"
#include <stdarg.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

bool or_strcmp(char *str, int count, ...);

void order_alphabetically(Move *moves, int num_moves);

#endif
