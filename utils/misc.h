#ifndef MAUTLIER_MISC_UTIL_H
#define MAUTLIER_MISC_UTIL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

bool or_strcmp(char *str, int count, ...);

#endif
