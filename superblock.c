#include "superblock.h"

int read_ext3_super_block(int fd, off_t offset, struct ext3_super_block *sb) {
	if (lseek(fd, offset, SEEK_SET) < 0) {
		return -1;
	}

	if (read(fd, sb, sizeof(struct ext3_super_block)) != sizeof(struct ext3_super_block)) {
		return -1;
	}

	if (sb->s_magic != EXT3_SUPER_MAGIC) {
		return -2;
	}

	return 0;
}
