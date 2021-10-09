#ifndef BLOCKS_H
#define BLOCKS_H

#include <stddef.h>
#include <sys/types.h>

#define CONSECUTIVE_BLOCK_THRESHOLD 3

ssize_t find_indirect_blocks(int fd, int **blocks);

#endif
