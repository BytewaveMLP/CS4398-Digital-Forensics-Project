#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200112L

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// EXT2/3/4 superblock magic number
#define EXT3_SUPER_MAGIC 0xef53

/**
 * EXT3 superblock structure
 *
 * (at least, all the fields that are interesting)
 */
struct ext3_super_block {
	uint32_t s_inodes_count;
	uint32_t s_blocks_count;
	uint32_t s_r_blocks_count;
	uint32_t s_free_blocks_count;
	uint32_t s_free_inodes_count;
	uint32_t s_first_data_block;
	uint32_t s_log_block_size;
	uint32_t s_log_frag_size;
	uint32_t s_blocks_per_group;
	uint32_t s_frags_per_group;
	uint32_t s_inodes_per_group;
	uint32_t s_mtime;
	uint32_t s_wtime;
	uint16_t s_mnt_count;
	uint16_t s_max_mnt_count;
	uint16_t s_magic;
	uint16_t s_state;
	uint16_t s_errors;
	uint16_t s_minor_rev_level;
	uint32_t s_last_check_time;
	uint32_t s_check_interval;
	uint32_t s_creator_os;
	uint32_t s_rev_level;
	uint16_t s_def_resuid;
	uint16_t s_def_resgid;
	uint32_t s_first_ino;
	uint16_t s_inode_size;
	uint16_t s_block_group_nr;
	uint32_t s_feature_compat;
	uint32_t s_feature_incompat;
	uint32_t s_feature_ro_compat;
	uint8_t s_uuid[16];
	uint8_t s_volume_name[16];
	uint8_t s_last_mounted[64];
	uint32_t s_algorithm_usage_bitmap;
	uint8_t s_prealloc_blocks;
	uint8_t s_prealloc_dir_blocks;
	uint16_t s_padding1;
	uint32_t s_reserved[204];
};

/**
 * Read an EXT3 superblock from the given byte location on disk
 *
 * @param file The block device to read from
 * @param offset The offset from the start of the disk to read from
 * @param sb The superblock to fill in
 * @return 0 on success, -1 on failure (check errno), -2 if the superblock is not valid
 */
int read_ext3_super_block(int fd, off_t offset, struct ext3_super_block *sb);

#endif
