#ifndef UTIL_H
#define UTIL_H

#ifdef DEBUG
#    define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#else
#    define debug_print(fmt, ...)
#endif

#endif
