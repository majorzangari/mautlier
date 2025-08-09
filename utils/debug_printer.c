#include "debug_printer.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define MAX_FLAGS 10
#define MAX_FLAG_LENGTH 50

static int flag_count = 0;
static char debug_flags[MAX_FLAGS][MAX_FLAG_LENGTH];

void dp_add_flag(const char *flag) {
  if (flag_count >= MAX_FLAGS) {
    fprintf(stderr, "Error: Maximum number of debug flags reached (%d).\n",
            MAX_FLAGS);
    return;
  }
  strncpy(debug_flags[flag_count++], flag, MAX_FLAG_LENGTH - 1);
}

void dp_printf(const char *flag, const char *format, ...) {
  for (int i = 0; i < flag_count; i++) {
    if (strncmp(debug_flags[i], flag, MAX_FLAG_LENGTH) == 0) {
      va_list args;
      va_start(args, format);
      vfprintf(stderr, format, args);
      va_end(args);
    }
  }
}
