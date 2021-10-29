/*
CS 4398.001 Digital Forensics
Project - Part 1
Group 22: Eliot Partridge
*/

#include "superblock.h"
#include "util.h"

#include <stdlib.h>
#define _POSIX_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

uint8_t riffSignature[4] = {'R', 'I', 'F', 'F'};
uint8_t aviSignature[4] = {'A', 'V', 'I', ' '};

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <device or image file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int devicefd;
	if ((devicefd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "Error: Could not open file %s: %s\n", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	struct ext3_super_block sb;

	int retval = read_ext3_super_block(devicefd, 1024, &sb);

	if (retval == -1) {
		printf("Could not read superblock: %s\n", strerror(errno));
		return EXIT_FAILURE;
	} else if (retval == -2) {
		printf("Could not read superblock: Invalid magic number\n");
		return EXIT_FAILURE;
	}

	uint64_t blockSize = 1024llu << sb.s_log_block_size;
	uint64_t blockCount = sb.s_blocks_count;
	uint8_t *blockData = malloc(blockSize);

	if (blockData == NULL) {
		printf("Could not allocate memory for block data\n");
		return EXIT_FAILURE;
	}

	for (uint64_t block = sb.s_first_data_block + 1; block < blockCount; block++) {
		off_t offset = block * blockSize;
		if (lseek(devicefd, offset, SEEK_SET) < 0) {
			printf("Could not seek to block %" PRIu64 ": %s\n", block, strerror(errno));
			return EXIT_FAILURE;
		}
		if (read(devicefd, blockData, blockSize) < 0) {
			printf("Could not read block %" PRIu64 ": %s\n", block, strerror(errno));
			return EXIT_FAILURE;
		}
		if (memcmp(blockData, riffSignature, 4) == 0) {
			debug_print("Found RIFF file at block %" PRIu64 "\n", block);
			if (memcmp(blockData + 8, aviSignature, 4) == 0) {
				printf("Found AVI file at block %" PRIu64 "\n", block);
			}
		}
	}

	return EXIT_SUCCESS;
}
