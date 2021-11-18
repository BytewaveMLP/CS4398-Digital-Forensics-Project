#define _LARGEFILE64_SOURCE 1
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "util.h"
#include "blocks.h"
#include "superblock.h"

ssize_t find_indirect_blocks(int fd, uint64_t **blocks) {
	struct ext3_super_block superblock;

	// read initial superblock, always located at position 1024
	long long retval;
	if ((retval = read_ext3_super_block(fd, 1024, &superblock)) < 0) {
		return retval;
	}

	uint64_t blockSize = 1024ll << superblock.s_log_block_size;
	uint64_t blockCount = superblock.s_blocks_count;

	debug_print("block size: %" PRIu64 " bytes\n", blockSize);
	debug_print("block count: %" PRIu64 " blocks\n", blockCount);

	size_t indirectBlockCount = 0;
	size_t indirectBlocksSize = 1;
	uint64_t *indirectBlocks = malloc(sizeof(uint64_t) * indirectBlocksSize);

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
		uint64_t maxConsecutiveBlockCount = 0;
		for (size_t i = 1; i < blockPointerCount; i++) {
			if (blockData[i] - blockData[i-1] != 1) {
				if (consecutiveBlockCount != 0 && consecutiveBlockCount < CONSECUTIVE_BLOCK_THRESHOLD) {
					// this block probably isn't indirect; the last run was too short
					debug_print("block %" PRIu64 " is probably not indirect\n"
								"\tconsecutiveBlockCount = %" PRIu64 " < %llu\n", block, consecutiveBlockCount, CONSECUTIVE_BLOCK_THRESHOLD);
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
			// this block probably is indirect
			debug_print("block %" PRIu64 " is probably indirect\n"
						"\tmaxConsecutiveBlockCount = %" PRIu64 " >= %llu\n", block, maxConsecutiveBlockCount, CONSECUTIVE_BLOCK_THRESHOLD);
			indirectBlocks[indirectBlockCount++] = block;

			if (indirectBlockCount == indirectBlocksSize) {
				indirectBlocksSize *= 2;
				debug_print("realloc: %zu bytes\n", indirectBlocksSize * sizeof(uint64_t));
				indirectBlocks = realloc(indirectBlocks, sizeof(uint64_t) * indirectBlocksSize);
			}
		}

		next_block:;
	}

	*blocks = indirectBlocks;
	return indirectBlockCount;
}
