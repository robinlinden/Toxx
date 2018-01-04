#ifndef MACROS_H
#define MACROS_H

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#else
#include <sys/param.h> // Provides MAX/MIN
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#else
#include <sys/param.h> // Provides MAX/MIN
#endif

// This is hacky and never better, use an alternative.
#define strcmp2(x, y) (memcmp(x, y, sizeof(y) - 1))
#define strcpy2(x, y) (memcpy(x, y, sizeof(y) - 1))

#define COUNTOF(x) (sizeof(x) / sizeof(*(x)))

// Wrap var in UNUSED(var) to correctly suppress warnings
#ifdef UNUSED
#undef UNUSED
#endif
#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) x
#endif

#endif
