#define _LARGEFILE64_SOURCE 1
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "util.h"
#include "blocks.h"
#include "superblock.h"

ssize_t find_indirect_blocks(int fd, int **blocks) {
	struct ext3_super_block superblock;

	// read initial superblock, always located at position 1024
	long long retval;
	if ((retval = read_ext3_super_block(fd, 1024, &superblock)) < 0) {
		return retval;
	}

	uint64_t blockSize = 1024ll << superblock.s_log_block_size;
	uint64_t blockCount = superblock.s_blocks_count;

	debug_print("block size: %llu bytes\n", blockSize);
	debug_print("block count: %llu blocks\n", blockCount);

	size_t indirectBlockCount = 0;
	size_t indirectBlocksSize = 1;
	int *indirectBlocks = malloc(sizeof(int) * indirectBlocksSize);

	for (uint64_t block = 1; block < blockCount; block++) {
		size_t blockPointerCount = blockSize / sizeof(uint32_t);
		uint32_t blockData[blockPointerCount];
		off64_t offset = block * blockSize;

		if ((retval = lseek64(fd, offset, SEEK_SET)) < 0) {
			return -3;
		}

		if ((retval = read(fd, blockData, blockSize)) < 0) {
			return -4;
		}

		uint64_t consecutiveBlockCount = 0;
		for (size_t i = 1; i < blockPointerCount; i++) {
			if (blockData[i] - blockData[i-1] != 1) {
				if (consecutiveBlockCount != 0 && consecutiveBlockCount < CONSECUTIVE_BLOCK_THRESHOLD) {
					// this block probably isn't indirect
					debug_print("block %llu is probably not indirect\n"
								"\tconsecutiveBlockCount = %llu < %llu\n", block, consecutiveBlockCount, CONSECUTIVE_BLOCK_THRESHOLD);
					goto next_block;
				}
				consecutiveBlockCount = 0;
				continue;
			}

			consecutiveBlockCount++;
		}

		if (consecutiveBlockCount >= CONSECUTIVE_BLOCK_THRESHOLD) {
			// this block probably is indirect
			debug_print("block %llu is probably indirect\n"
						"\tconsecutiveBlockCount = %llu >= %llu\n", block, consecutiveBlockCount, CONSECUTIVE_BLOCK_THRESHOLD);
			indirectBlocks[indirectBlockCount++] = block;

			if (indirectBlockCount == indirectBlocksSize) {
				indirectBlocksSize *= 2;
				debug_print("realloc: %zu bytes\n", indirectBlocksSize * sizeof(int));
				indirectBlocks = realloc(indirectBlocks, sizeof(int) * indirectBlocksSize);
			}
		}

		next_block:;
	}

	*blocks = indirectBlocks;
	return indirectBlockCount;
}
