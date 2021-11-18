#ifndef UTIL_H
#define UTIL_H

#ifdef DEBUG
#    define debug_print(fmt, ...) \
            do { if (DEBUG) printf(fmt, __VA_ARGS__); } while (0)
#else
#    define debug_print(fmt, ...)
#endif

void hexdump(const void *data, size_t size);

#endif
