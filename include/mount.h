#ifndef MOUNT_H
#define MOUNT_H

#define FUSE_USE_VERSION 31

#include <definitions.h>
#include <filesystem.h>

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
// For user info
#include <unistd.h>
#include <sys/types.h>
// For setting errors
#include <errno.h>

int mount_archive(char* mountpoint, char* archive_path);

// Callbacks for libfuse
static int arch_read(const char *path, char *buf, size_t size, off_t offset,
		   struct fuse_file_info *fi);
static int arch_open(const char *path, struct fuse_file_info *fi);
static int arch_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags);
static int arch_getattr(const char *path, struct stat *stbuf,
       struct fuse_file_info *fi);
static void *arch_init(struct fuse_conn_info *conn,
       struct fuse_config *cfg);

#endif
