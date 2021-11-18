/*
CS 4398.001 Digital Forensics
Project - Part 1
Group 22: Eliot Partridge
*/

#include "superblock.h"
#include "util.h"
#include "blocks.h"

#include <stdlib.h>
#define _POSIX_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <stdbool.h>

uint8_t riffSignature[4] = {'R', 'I', 'F', 'F'};
uint8_t aviSignature[4] = {'A', 'V', 'I', ' '};

uint8_t interestingMarkers[] = {
	'L', 'I', 'S', 'T',
	'h', 'd', 'r', 'l',
	'a', 'v', 'i', 'h',
	's', 't', 'r', 'l',
	's', 't', 'r', 'h',
	's', 't', 'r', 'f',
	's', 't', 'r', 'd',
	'm', 'o', 'v', 'i',
	'0', '0', 'd', 'b',
	'0', '0', 'd', 'c',
	'0', '0', 'p', 'c',
	'0', '0', 'w', 'b',
	'i', 'd', 'x', '1',
};
size_t markerCount = sizeof(interestingMarkers) / 4;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <device or image file> <file to write recovered data to>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int devicefd;
	if ((devicefd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "Error: Could not open file %s: %s\n", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	int outfd;
	if ((outfd = open(argv[2], O_WRONLY | O_CREAT, 0664)) < 0) {
		fprintf(stderr, "Error: Could not open file %s: %s\n", argv[2], strerror(errno));
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
	uint8_t blockData[blockSize];

	uint32_t expectedFileSize = 0;
	uint64_t foundFileSize = 0;
	uint64_t aviFileStartBlock = 0;

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
				expectedFileSize = *(uint32_t *)(blockData + 4) + 8;
				aviFileStartBlock = block;
				printf("Found AVI file header at block %" PRIu64 " w/ declared size %" PRIu32 "\n", aviFileStartBlock, expectedFileSize);
				// we only need to recover one file...
				break;
			}
		}
	}

	if (aviFileStartBlock == 0) {
		printf("Couldn't find any AVI files to recover\n");
		return EXIT_FAILURE;
	}

	if (write(outfd, blockData, blockSize) == -1) {
		printf("Could not write block %" PRIu64 ": %s\n", aviFileStartBlock, strerror(errno));
		return EXIT_FAILURE;
	}

	uint64_t *indirectBlocks = NULL;
	ssize_t indirectBlockCount = find_indirect_blocks(devicefd, &indirectBlocks);
	if (indirectBlockCount < 0) {
		fprintf(stderr, "Error: Failed to read disk: ");
		fprintf(stderr, "(%zd): %s\n", indirectBlockCount, strerror(errno));

		return EXIT_FAILURE;
	}

	uint8_t indirectBlockData[blockSize];
	bool lastBlock = false;
	uint64_t idx1Size = 0;
	uint64_t idx1Offset = 0;

	debug_print("Found %zd indirect blocks\n", indirectBlockCount);
	for (ssize_t i = 0; i < indirectBlockCount; i++) {
		uint64_t block = indirectBlocks[i];

		debug_print("Reading block %" PRIu64 "\n", block);

		off_t offset = block * blockSize;
		if (lseek(devicefd, offset, SEEK_SET) < 0) {
			printf("Could not seek to block %" PRIu64 ": %s\n", block, strerror(errno));
			return EXIT_FAILURE;
		}
		if (read(devicefd, blockData, blockSize) < 0) {
			printf("Could not read block %" PRIu64 ": %s\n", block, strerror(errno));
			return EXIT_FAILURE;
		}

		for (uint64_t indirectBlockOffset = 0; indirectBlockOffset < blockSize; indirectBlockOffset += 4) {
			uint32_t *blockOffset = (uint32_t *)(blockData + indirectBlockOffset);
			if (*blockOffset == 0) break;

			debug_print("Scanning block %" PRIu32 " (indirectly referred to by %" PRIu64 ")\n", *blockOffset, block);

			if (lseek(devicefd, *blockOffset * blockSize, SEEK_SET) < 0) {
				printf("Could not seek to block %" PRIu64 ": %s\n", block, strerror(errno));
				return EXIT_FAILURE;
			}
			if (read(devicefd, indirectBlockData, blockSize) < 0) {
				printf("Could not read block %" PRIu64 ": %s\n", block, strerror(errno));
				return EXIT_FAILURE;
			}

			unsigned long int foundMarkerCount = 0;

			for (size_t i = 0; i < markerCount; i++) {
				uint8_t *marker = interestingMarkers + (i * 4);
				for (uint64_t j = 0; j < blockSize; j++) {
					if (memcmp(marker, indirectBlockData + j, 4) == 0) {
						debug_print("Found marker %c%c%c%c at block %" PRIu64 " offset %" PRIu64 "\n", marker[0], marker[1], marker[2], marker[3], block, j);
						foundMarkerCount++;
						if (i == markerCount - 1) {
							// idx1
							debug_print("Found idx1 at block %" PRIu64 " offset %" PRIu64 "\n", block, j);

							idx1Offset = j;
							idx1Size = *(uint32_t *)(indirectBlockData + j + 4);

							debug_print("idx1 size is %" PRIu64 "\n", idx1Size);

							lastBlock = true;
							break;
						}
					}
				}
			}

			debug_print("Found %lu interesting markers\n", foundMarkerCount);

			if (foundMarkerCount > 0) {
				debug_print("Saving potential AVI file block %" PRIu32 "...\n", *blockOffset);

				uint64_t dataSize = blockSize;

				if (lastBlock) {
					dataSize = idx1Offset + idx1Size + 4;
				}

				if (write(outfd, indirectBlockData, dataSize) == -1) {
					printf("Could not write block %" PRIu32 ": %s\n", *blockOffset, strerror(errno));
					return EXIT_FAILURE;
				}
				foundFileSize += dataSize;
				debug_print("Expected file size is now %" PRIu32 "\n", expectedFileSize);

				if (lastBlock) goto end;
			}
		}
	}

	end:;

	int64_t diff = expectedFileSize - foundFileSize;

	printf("Recovered %" PRIu64 "/%" PRIu32 " bytes of AVI file (diff=%" PRId64 ")\n", foundFileSize, expectedFileSize, diff);

	return EXIT_SUCCESS;
}
