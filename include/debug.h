#pragma once

#include <stdarg.h>

#ifdef DEBUG_MSG
#define DEBUGF(fmt, ...) fprintf(stderr, "[DEBUG][%s]: "fmt"\n", __func__, __VA_ARGS__)
#define DEBUG(msg) fprintf(stderr, "[DEBUG][%s]: "msg"\n", __func__)
#else
#define DEBUGF(fmt, ...)
#define DEBUG(msg)
#endif


