#include "blocks.h"
#include "superblock.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

size_t find_indirect_blocks(int fd, int **blocks) {
	struct ext3_super_block superblock;

	// read initial superblock, always located at position 1024
	int retval = read_ext3_super_block(fd, 1024, &superblock);

	if (retval == -1) {
		printf("Could not read superblock: %s\n", strerror(errno));
		return EXIT_FAILURE;
	} else if (retval == -2) {
		printf("Could not read superblock: Invalid magic number\n");
		return EXIT_FAILURE;
	}

	uint64_t blockSize = 1024ll << superblock.s_log_block_size;
	uint64_t blockCount = superblock.s_blocks_count;

	size_t indirectBlockCount = 0;
	size_t indirectBlocksSize = 1;
	int *indirectBlocks = malloc(sizeof(int) * indirectBlocksSize);

	for (uint64_t block = 1; block < blockCount; block++) {
		size_t blockPointerCount = blockSize / sizeof(uint32_t);
		uint32_t blockData[blockPointerCount];
		uint64_t offset = block * blockSize;

		int retval;
		if ((retval = lseek(fd, offset, SEEK_SET)) < 0) {
			return -1;
		}

		if ((retval = read(fd, blockData, blockSize)) < 0) {
			return -1;
		}

		uint64_t consecutiveBlockCount = 0;
		uint64_t maxConsecutiveBlockCount = 0;
		for (size_t i = 1; i < blockPointerCount; i++) {
			if (blockData[i] - blockData[i-1] != 1) {
				if (consecutiveBlockCount != 0 && consecutiveBlockCount < CONSECUTIVE_BLOCK_THRESHOLD) {
					// this block probably isn't indirect; the last run was too short
					goto next_block;
				}
				if (consecutiveBlockCount > maxConsecutiveBlockCount) {
					maxConsecutiveBlockCount = consecutiveBlockCount;
				}
				consecutiveBlockCount = 0;
				continue;
			}

			consecutiveBlockCount++;
		}

		if (maxConsecutiveBlockCount >= CONSECUTIVE_BLOCK_THRESHOLD) {
			indirectBlocks[indirectBlockCount++] = block;

			if (indirectBlockCount == indirectBlocksSize) {
				indirectBlocksSize *= 2;
				indirectBlocks = realloc(indirectBlocks, sizeof(int) * indirectBlocksSize);
			}
		}

		next_block:;
	}

	*blocks = indirectBlocks;
	return indirectBlockCount;
}
