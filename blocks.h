#ifndef BLOCKS_H
#define BLOCKS_H

#include <stddef.h>
#include <sys/types.h>

#define CONSECUTIVE_BLOCK_THRESHOLD 3ULL

ssize_t find_indirect_blocks(int fd, uint64_t **blocks);

#endif
