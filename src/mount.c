#include <mount.h>

// Code to be executed first when the filesystem is loaded
static void*
arch_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

// Code used to retrieve attributes from a file
static int
arch_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;

	struct file_entry* file = get_file(path);
	if (file == NULL){
		errno = ENOENT;
		return -ENOENT;
	}


	memset(stbuf, 0, sizeof(struct stat));
	stbuf->st_uid = owner;
	stbuf->st_gid = owner_group;
	stbuf->st_mode = S_IRUSR | S_IRGRP;
	if (file->is_folder) {
		stbuf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP;
		stbuf->st_nlink = 1;
	}
	else {
		stbuf->st_mode |= S_IFREG;
		stbuf->st_nlink = 1;
		stbuf->st_size = file->size;
	}

	return 0;
}

// Code used to read the entries of a directory
static int
arch_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	struct file_entry* file = get_file(path);
	if(file == NULL) {
		errno = ENOENT;
		return -ENOENT;
	}

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	for (struct file_entry* it = file->folder_descendent; it != NULL; it = it->brother_link) filler(buf, it->name, NULL, 0, 0);
	for (struct file_entry* it = file->file_descendent; it != NULL; it = it->brother_link) filler(buf, it->name, NULL, 0, 0);

	return 0;
}

// Code to open a file
static int
arch_open(const char *path, struct fuse_file_info *fi)
{
	struct file_entry* file = get_file(path);
	if (file == NULL) {
		errno = ENOENT;
		return -ENOENT;
	}

	if ((fi->flags & O_ACCMODE) != O_RDONLY) {
		errno = EACCES;
		return -EACCES;
	}

	int result = open_file(fi->fh, file->index);
	if(result != 0) {
		errno = ECANCELED;
		return -ECANCELED;
	}

	return 0;
}

// Code to read from an opened file
static int
arch_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	struct file_entry* file = get_file(path);
	if (file == NULL) {
		errno = ENOENT;
		return -ENOENT;
	}

	struct archive_node* archive_node = search_node(fi->fh);
	// File should be already opened when trying to read from it
	if (file == NULL) {
		errno = EBADF;
		return -EBADF;
	}

	size = archive_read_data(archive_node->archive, buf, size);

	return size;
}

// Code used when closing a file
int arch_release(const char *path, struct fuse_file_info *fi)
{
	close_file(fi->fh);
}

// Code executed when unmounting the archive
void arch_destroy(void *userdata)
{
  free_filesystem();
}

// Struct to associate operations provided by libfuse with their implementations provided in this module
static struct fuse_operations archive_oper = {
  .init     =		arch_init,
  .getattr	= 	arch_getattr,
  .readdir	= 	arch_readdir,
  .open			= 	arch_open,
	.release  =   arch_release,
  .read			= 	arch_read,
	.destroy  =		arch_destroy,
};

int
mount_archive(char* mountpoint, char* archive_path) {
	// Getting the user data
	owner = getuid();
	owner_group = getgid();
	// Prepare the archive for fuse use
	int archive_status = init_archive(archive_path);
	if (archive_status != 0) return archive_status;

	// Init args
	int argc = 2;
	char *argv[] = {
		(char*)"fuse",
    mountpoint,
    NULL
	};
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	if (verbose_flag) printf("Mounting the filesystem...\n");

  // Init fuse
	if (fuse_opt_parse(&args, NULL, NULL, NULL) == -1) return UNEXPECTED_ERROR;
  int ret = fuse_main(args.argc, args.argv, &archive_oper, NULL);
  fuse_opt_free_args(&args);
  return ret;
}
