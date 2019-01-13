 #include <filesystem.h>

// Add to the stack of files in the fs
static void
add_file_node(struct file_entry* node) {
   last_entry->entry = node;
   last_entry->next = malloc(sizeof(struct file_node));
   last_entry = last_entry->next;
   memset(last_entry, 0, sizeof(struct file_node));
}

// The procedure to add new files to fs
static void
add_file_to_entry(char* name, struct file_entry* father, struct archive_entry* file_header, int index) {
  // Creating a new entry for the file
  struct file_entry* new_file = (struct file_entry*) malloc(sizeof(struct file_entry));
  memset(new_file, 0, sizeof(struct file_entry));
  new_file->name = name;
  new_file->index = index;
  new_file->father = father;
  // Check if it is a file rather than a folder
  int type = archive_entry_filetype(file_header);
  // Check if it is a file rather than a folder
  if (type == AE_IFREG) {
    new_file->size = archive_entry_size(file_header);
    // If it is the first file linked to parent
    if(father->file_descendent == NULL) {
      father->file_descendent = new_file;
      new_file->last_brother_in_list = new_file;
    }
    else {
      father->file_descendent->last_brother_in_list->brother_link = new_file;
      father->file_descendent->last_brother_in_list = new_file;
    }
  }
  else {
    new_file->is_folder = 1;
    // If it is the first folder linked to parent
    if(father->folder_descendent == NULL) {
      father->folder_descendent = new_file;
      new_file->last_brother_in_list = new_file;
    }
    else {
      father->folder_descendent->last_brother_in_list->brother_link = new_file;
      father->folder_descendent->last_brother_in_list = new_file;
    }
  }
  // Add this file to the stack of files
  add_file_node(new_file);
}

// Process the path to the files in fs and then add them
// Index if for the header count in the archive
static void
add_entry (struct archive_entry* arch_entry, int index) {
  char* path = strdup(archive_entry_pathname(arch_entry));
  // String with the current part of the path
  char* current_entry_path = NULL;
  struct file_entry *iterator, *aux;
  iterator = NULL;

  char* part = strtok(path, "/");
  // Process path until reach the file or final directory
  while (part != NULL) {
    // If the iterator points to a folder
    if (iterator != NULL) {
      struct file_entry* next_folder = NULL;
      // Search if the folder already exists in the filesystem
      for (struct file_entry *it = iterator->folder_descendent; it != NULL; it = it->brother_link)
        if (strcmp(it->name, current_entry_path) == 0) {
          next_folder = it;
          break;
        }

      // Create a new folder if necessary
      // It should never create a new folder here because of the default order of file headers, but maybe some archive formats do not preserve such an order
      if(next_folder == NULL) {
        add_file_to_entry(current_entry_path, iterator, arch_entry, index);
      }
      else {
        iterator = next_folder;
        free(current_entry_path);
      }
    }
    else iterator = main_entry;

    current_entry_path = strdup(part);
    part = strtok(NULL, "/");
  }

  // Now that the path is processed till the last part add the wanted file to filesystem
  add_file_to_entry(current_entry_path, iterator, arch_entry, index);

  free(path);
}

// Open files from the fs
int
open_file(int fd, int index) {
  struct archive *arch = archive_read_new();
  struct archive_entry *entry;
  archive_read_support_format_all(arch);
  archive_read_support_filter_all(arch);

  int r;
  // Open the archive again
  if ((r = archive_read_open_filename(arch, path_to_archive, 10240))) {
    fprintf(stderr, "%s\n", archive_error_string(arch));
    return ARCHIVE_ERROR;
  }

  // Make the archive ready for reading
  for (int i = 0; i <= index; i++) {
    r = archive_read_next_header(arch, &entry);
    if (r == ARCHIVE_EOF)
      break;
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "%s\n", archive_error_string(arch));
        return ARCHIVE_ERROR;
    }
  }

  // Store the archive in a node to acces it later for reading
  struct archive_node* arch_node = (struct archive_node*) malloc(sizeof(struct archive_node));
  memset(arch_node, 0, sizeof(struct archive_node));
  arch_node->archive = arch;
  // TODO not sure if there is anything else to do
  if (first_archive_node == NULL) {
    first_archive_node = last_archive_node = arch_node;
  }
  else {
    last_archive_node->next = arch_node;
    last_archive_node = arch_node;
  }

  return 0;
}

// Search for a node with an archive pointer
// Useful for reading
struct archive_node*
search_node(uint64_t fd) {
  for (struct archive_node* it = first_archive_node; it != NULL; it = it->next)
    if (it->file_handle == fd) return it;
  return NULL;
}

// Used to provide acces to file headers, returns null when file not found
struct file_entry*
get_file(const char* path) {
  struct file_entry* iterator = main_entry;
	char* path_copy = strdup(path);
  // Process the path
	char* part = strtok(path_copy, "/");
	while (part != NULL) {
		int found = 0;
		for (struct file_entry* it = iterator->folder_descendent; it != NULL; it = it->brother_link)
			if (strcmp(it->name, part) == 0) {
				iterator = it;
				found = 1;
				break;
			}
    // Process next part of the path
    char* last_path_part = strdup(part);
		part = strtok(NULL, "/");
    if (part == NULL) {
      for (struct file_entry* it = iterator->file_descendent; it != NULL; it = it->brother_link)
  			if (strcmp(it->name, last_path_part) == 0) {
  				iterator = it;
  				found = 1;
  				break;
  			}
    }

    // The current file or folder processed from path does not exist in the filesystem so that means the path is invalid
		if (found == 0) {
			free(path_copy);
			return NULL;
		}
    free(last_path_part);
	}

  free(path_copy);
  return iterator;
}

// Close a file opened by open_file
void
close_file(int fd) {
  struct archive_node* searched_node = NULL;
  struct archive_node* node_before = NULL;
  // Search for the node
  for (struct archive_node* it = first_archive_node; it != NULL; node_before = it, it = it->next)
    if (it->file_handle == fd) {
      searched_node = it;
      break;
    }
  // If the fd still in list
  if (searched_node != NULL) {
    // If the node is not the first in the list
    if (node_before != NULL)
      node_before->next = searched_node->next;
    else
      first_archive_node = searched_node->next;
    // If the node is the last in the list
    if (searched_node == last_archive_node)
      last_archive_node = node_before;
    // Finally free memory
    free(searched_node);
  }
}

// Free the memory used for creating the filesystem
void
free_filesystem () {
  // Free the list of files
  struct file_node* it = first_entry;
  struct file_node* aux;
  while (it != NULL) {
    free(it->entry);
    aux = it;
    it = it->next;
    free(aux);
  }
  free(path_to_archive);
  // Free the archives file descriptors
  struct archive_node* arch_it = first_archive_node;
  while (arch_it != NULL) {
    archive_read_close(arch_it->archive);
    archive_read_free(arch_it->archive);
    struct archive_node* aux = arch_it->next;
    free(arch_it);
    arch_it = aux;
  }
}

int
init_archive (char* path) {
  if (verbose_flag) printf("Loading the archive...\n");
  path_to_archive = strdup(path);
  struct archive *arch = archive_read_new();
  struct archive_entry *entry;
  archive_read_support_format_all(arch);
  archive_read_support_filter_all(arch);
  first_archive_node = last_archive_node = NULL;

  // Index is used to know where to search for the file when content requested
  int index = 0;
  int r;

  if ((r = archive_read_open_filename(arch, path, 10240))) {
    fprintf(stderr, "%s\n", archive_error_string(arch));
    return ARCHIVE_ERROR;
  }

  main_entry = (struct file_entry*) malloc(sizeof(struct file_entry));
  memset(main_entry, 0, sizeof(struct file_entry));
  main_entry->is_folder = 1;

  first_entry = last_entry = (struct file_node*) malloc(sizeof(struct file_node));
  memset(first_entry, 0, sizeof(struct file_node));

  // Read details about each file and put them into the filesystem
  if (verbose_flag) printf("Building the filesystem...\n");
  for (;;) {
		r = archive_read_next_header(arch, &entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r != ARCHIVE_OK) {
        fprintf(stderr, "%s\n", archive_error_string(arch));
        return ARCHIVE_ERROR;
    }
    // Add the current file header to filesystem
    add_entry(entry, index);
    index++;
  }

  archive_read_close(arch);
  archive_read_free(arch);
  return 0;
}
