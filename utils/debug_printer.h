#ifndef MAUTLIER_DEBUG_PRINTER_H
#define MAUTLIER_DEBUG_PRINTER_H

// Flags currently used in the codebase:
// FUNC_TRACE - traces function calls

// SHOULD NOT BE CALLED ON ITS OWN, SHOULD USE ASSOCIATED MACROS TO NOT WASTE
// PERFORMANCE ON NON-DEBUG BUILDS
void dp_add_flag(const char *flag);

// SHOULD NOT BE CALLED ON ITS OWN, SHOULD USE ASSOCIATED MACROS TO NOT WASTE
// PERFORMANCE ON NON-DEBUG BUILDS
void dp_printf(const char *flag, const char *format, ...);

#ifdef MAUTLIER_DEBUG
#define DP_ADD_FLAG(flag) dp_add_flag(flag)
#define DP_PRINTF(flag, ...) dp_printf(flag, __VA_ARGS__)
#else
#define DP_ADD_FLAG(flag) ((void)0)
#define DP_PRINTF(flag, ...) ((void)0)
#endif

#endif
