/*
CS 4398.001 Digital Forensics
Project - Part 1
Group 8: Eliot Partridge, Kent Templin
*/

#include "superblock.h"

#include <stdlib.h>
#define _POSIX_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define compare_sb_field(sb1, sb2, field) compare_sb_field_fmt(sb1, sb2, field, "%u")

#define compare_sb_field_fmt(sb1, sb2, field, fmt) \
	if (sb1->field != sb2->field) { \
		printf("DIFF - " fmt " != " fmt "\n", sb1->field, sb2->field); \
		result = 0; \
	} else { \
		printf("PASS - " fmt "\n", sb2->field); \
	}

static uint32_t backupSuperblockLocations[] = { 1, 3, 5, 7, 9, 25, 27, 49, 81, 125, 243, 343, 625, 729, 2401, 3125, 15625, 16807, };
static size_t backupSuperblockLocationsLen = sizeof(backupSuperblockLocations) / sizeof(backupSuperblockLocations[0]);

/**
 * Prints useful details about a given superblock
 *
 * @param sb The superblock to print details for
 */
void print_superblock(struct ext3_super_block *sb) {
	printf("\tInode count: %u\n", sb->s_inodes_count);
	printf("\tBlock count: %u\n", sb->s_blocks_count);
	printf("\tReserved block count: %u\n", sb->s_r_blocks_count);
	printf("\tFree block count: %u\n", sb->s_free_blocks_count);
	printf("\tFree inode count: %u\n", sb->s_free_inodes_count);
	printf("\tFirst data block: %u\n", sb->s_first_data_block);
	printf("\tlog(Block size): %u\n", sb->s_log_block_size);
	// printf("\tFragment size: %llu\n", 1024ll << sb->s_log_frag_size);
	printf("\tBlocks per group: %u\n", sb->s_blocks_per_group);
	// printf("\tFragments per group: %u\n", sb->s_frags_per_group);
	printf("\tInodes per group: %u\n", sb->s_inodes_per_group);
	printf("\tLast mount time: %u\n", sb->s_mtime);
	printf("\tLast write time: %u\n", sb->s_wtime);
	printf("\tMount count: %u\n", sb->s_mnt_count);
	printf("\tMax mount count: %u\n", sb->s_max_mnt_count);
	printf("\tMagic number: %x\n", sb->s_magic);
	// printf("\tState: %u\n", sb->s_state);
	printf("\tErrors: %u\n", sb->s_errors);
	// printf("\tMinor revision level: %u\n", sb->s_minor_rev_level);
	// printf("\tLast check time: %u\n", sb->s_last_check_time);
	// printf("\tCheck interval: %u\n", sb->s_check_interval);
	printf("\tOS: %u\n", sb->s_creator_os);
	printf("\tRevision: %u\n", sb->s_rev_level);
	printf("\tDefault UID: %u\n", sb->s_def_resuid);
	printf("\tDefault GID: %u\n", sb->s_def_resgid);
}

int compare_superblocks(struct ext3_super_block *sb1, struct ext3_super_block *sb2) {
	int result = 1;

	printf("\tInode count: ");
	compare_sb_field(sb1, sb2, s_inodes_count);

	printf("\tBlock count: ");
	compare_sb_field(sb1, sb2, s_blocks_count);

	printf("\tReserved block count: ");
	compare_sb_field(sb1, sb2, s_r_blocks_count);

	printf("\tFree block count: ");
	compare_sb_field(sb1, sb2, s_free_blocks_count);

	printf("\tFree inode count: ");
	compare_sb_field(sb1, sb2, s_free_inodes_count);

	printf("\tFirst data block: ");
	compare_sb_field(sb1, sb2, s_first_data_block);

	printf("\tlog(Block size): ");
	compare_sb_field(sb1, sb2, s_log_block_size);

	printf("\tBlocks per group: ");
	compare_sb_field(sb1, sb2, s_blocks_per_group);

	printf("\tInodes per group: ");
	compare_sb_field(sb1, sb2, s_inodes_per_group);

	printf("\tLast mount time: ");
	compare_sb_field(sb1, sb2, s_mtime);

	printf("\tLast write time: ");
	compare_sb_field(sb1, sb2, s_wtime);

	printf("\tMount count: ");
	compare_sb_field(sb1, sb2, s_mnt_count);

	printf("\tMax mount count: ");
	compare_sb_field(sb1, sb2, s_max_mnt_count);

	// printf("\tState: ");
	// compare_sb_field(sb1, sb2, s_state);

	printf("\tErrors: ");
	compare_sb_field(sb1, sb2, s_errors);

	printf("\tOS: ");
	compare_sb_field(sb1, sb2, s_creator_os);

	printf("\tRevision: ");
	compare_sb_field(sb1, sb2, s_rev_level);

	printf("\tDefault UID: ");
	compare_sb_field(sb1, sb2, s_def_resuid);

	printf("\tDefault GID: ");
	compare_sb_field(sb1, sb2, s_def_resgid);

	return result;
}

int main(int argc, char *argv[]) {
	// PART 1 - Read EXT3 superblock and print important details
	struct ext3_super_block superblock;

	if (argc != 2) {
		printf("Usage: %s <device or image file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	FILE *device = fopen(argv[1], "r");
	if (device == NULL) {
		printf("Error: Could not open file %s: %s\n", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	// read initial superblock, always located at position 1024
	int retval = read_ext3_super_block(device, 1024, &superblock);

	if (retval == -1) {
		printf("Could not read superblock: %s\n", strerror(errno));
		return EXIT_FAILURE;
	} else if (retval == -2) {
		printf("Could not read superblock: Invalid magic number\n");
		return EXIT_FAILURE;
	}

	// print superblock information
	printf("Superblock information:\n");
	print_superblock(&superblock);

	// PART 2 - Read backup superblocks and compare with primary superblock
	uint64_t blockSize = 1024ll << superblock.s_log_block_size;
	uint64_t blockCount = superblock.s_blocks_count;
	uint64_t blockGroupSize = superblock.s_blocks_per_group;
	uint64_t blockGroupCount = blockCount / superblock.s_blocks_per_group;

	for (size_t i = 0; i < backupSuperblockLocationsLen; i++) {
		printf("\n");

		uint64_t location = backupSuperblockLocations[i];
		if (location >= blockGroupCount) {
			break;
		}

		struct ext3_super_block backupSuperblock;
		retval = read_ext3_super_block(device, location * blockGroupSize * blockSize, &backupSuperblock);

		if (retval == -1) {
			printf("Could not read backup superblock %lu: %s\n", location, strerror(errno));
			return EXIT_FAILURE;
		} else if (retval == -2) {
			printf("Could not read backup superblock %lu: Invalid magic number\n", location);
			continue;
		}

		printf("Comparing superblocks at blockgroup 0 and blockgroup %lu\n", location);
		compare_superblocks(&superblock, &backupSuperblock);
	}

	return EXIT_SUCCESS;
}
