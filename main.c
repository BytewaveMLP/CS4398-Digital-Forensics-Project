/*
CS 4398.001 Digital Forensics
Project - Part 1
Group 22: Eliot Partridge
*/

#include "superblock.h"
#include "blocks.h"

#include <stdlib.h>
#define _POSIX_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <device or image file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int devicefd;
	if ((devicefd = open(argv[1], O_RDONLY)) < 0) {
		printf("Error: Could not open file %s: %s\n", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	int *indirectBlocks;
	size_t indirectBlocksCount = find_indirect_blocks(devicefd, &indirectBlocks);

	printf("Found %zd indirect blocks\n", indirectBlocksCount);
	for (size_t i = 0; i < indirectBlocksCount; i++) {
		printf("%d", indirectBlocks[i]);
		if (i != indirectBlocksCount - 1) {
			printf(", ");
		}
	}
	printf("\n");

	return EXIT_SUCCESS;
}
