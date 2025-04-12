#pragma once

#include <stdarg.h>

#ifdef DEBUG_MSG
#define DEBUGF(lvl, fmt, ...) fprintf(stderr, "[DEBUG][%s]: "fmt"\n", __func__, __VA_ARGS__)
#define DEBUG(lvl, msg) fprintf(stderr, "[DEBUG][%s]: "msg"\n", __func__)
#else
#define DEBUGF(lvl, fmt, ...)
#define DEBUG(lvl, msg)
#endif


