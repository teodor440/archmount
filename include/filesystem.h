#ifndef ARCH_H
#define ARCH_H

#include <definitions.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Struct for the file system
struct file_entry {
  char* name;
  long size;
  // Flag for whether this file is a folder or not
  int is_folder;
  // The index in the archive for this file
  int index;
  // Need a refference for ..
  struct file_entry* father;
  // Files have links to the next ones in the current folder
  struct file_entry* brother_link;
  // Last brother used for easier new file/folders appending
  struct file_entry* last_brother_in_list;
  // If this file is a folder it should have pointers to descending sons
  struct file_entry* folder_descendent;
  struct file_entry* file_descendent;
};


// Struct used for opened files
struct archive_node {
  struct archive_node* next;
  struct archive* archive;
  uint64_t file_handle;
};


// Struct to retain entries to delete them later
struct file_node {
  struct file_node* next;
  struct file_entry* entry;
};

static char* path_to_archive;

// Search for a node with an archive pointer
// Useful for reading
struct archive_node* search_node(uint64_t fd);
// Add to the stack of files in the fs
static void add_file_node(struct file_entry* node);
// The procedure to add new files to fs
static void add_file_to_entry(char* name, struct file_entry* father, struct archive_entry* file_header, int index);
// Process the path to the files in fs and then add them
// Index if for the header count in the archive
static void add_entry (struct archive_entry* arch_entry, int index);

// Open files from the fs
int open_file(int fd, int index);
// Closes a file from the fs
void close_file(int fd);
// Used to provide acces to file headers, returns null when file not found
struct file_entry* get_file(const char* path);
// Search for a node with an archive pointer
// Useful for reading
struct archive_node* search_node(uint64_t fd);
// Free the filesystem
void free_filesystem ();

// Called to create the filesystem for that archive
int init_archive (char* path);

#endif
