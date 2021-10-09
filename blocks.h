#ifndef BLOCKS_H
#define BLOCKS_H

#include <stddef.h>

#define CONSECUTIVE_BLOCK_THRESHOLD 3

size_t find_indirect_blocks(int fd, int **blocks);

#endif
