#include <stdio.h>

#include "superblock.h"

int read_ext3_super_block(FILE *file, int offset, struct ext3_super_block *sb) {
	if (fseek(file, offset, SEEK_SET) != 0) {
		return -1;
	}

	if (fread(sb, sizeof(struct ext3_super_block), 1, file) != 1) {
		return -1;
	}

	if (sb->s_magic != EXT3_SUPER_MAGIC) {
		return -2;
	}

	return 0;
}
