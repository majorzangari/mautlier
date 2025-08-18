#include "misc.h"

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
