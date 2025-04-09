#pragma once

#include <stdarg.h>

#ifdef DEBUG_MSG
#define DEBUGF(fmt, ...) fprintf(stderr, "[DEBUG]: "fmt"\n", __VA_ARGS__)
#define DEBUG(msg) fprintf(stderr, "[DEBUG]: "msg"\n")
#else
#define DEBUGF(fmt, ...)
#define DEBUG(msg)
#endif
