#!/bin/sh

set -ex

[ -f test-disk-ext2 ] && rm test-disk-ext2
fallocate -l 2G test-disk-ext2
mkfs.ext2 -F test-disk-ext2
sudo mount test-disk-ext2 test-disk
