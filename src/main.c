#include <definitions.h>
#include <mount.h>
// For parsing options
#include <getopt.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

int verbose_flag = 0;

static void
show_help() {
	printf("usage: %s [options] <mountpoint> <archive_path>\n\n", __PROGRAM_NAME);
	printf("File-system specific options:\n"
	       "    -h                  Help\n"
	       "    -v                  Verbose\n"
				 "    --verbose           Verbose\n"
				 "    --version           Shows version\n"
	       "    --help              Shows usage\n"
	       "\n");
}

int
main(int argc, char *argv[])
{
	// Reading parameters
	opterr = 0;
  while (1)
    {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"verbose",  	       no_argument,     &verbose_flag,        1},
					{"version",       	 no_argument,     0,                    0},
					{"help",       	 		 no_argument,     0, 										0},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"verbose",     no_argument,       0, 'v'},
          {"show help",    no_argument,       0, 'h'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

			// "vh" are the short options
      int option = getopt_long (argc, argv, "vh",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (option == -1)
        break;

			// Process the current options
      switch (option)
        {
        case 0:
          /* When long option processed
 						 It is stored in optarg the argument of the option */
          if (long_options[option_index].flag != 0)
            break;
          if(strcmp(long_options[option_index].name, "version") == 0) {
						printf("%s\n", __VERSION);
						return 0;
					}
					else if(strcmp(long_options[option_index].name, "help") == 0) {
						show_help();
						return 0;
					}
          break;

        case 'v':
          verbose_flag = 1;
          break;

        case 'h':
          show_help();
					return 0;
          break;

        case '?': // When no paramter provided, consider also the case with ':'
					fprintf(stderr, "There is no such parameter as %c\n", optopt);
					show_help();
					return WRONG_PARAMS;
          break;

        default:
          fprintf(stderr, "Unexpected error while processing args\n");
					return WRONG_PARAMS;
        }
    }

  /* Print any remaining command line arguments (not options). */
	char* mountpoint;
	char* archive_path;

	if (verbose_flag) printf("Solving the arguments...\n");

	if (optind < argc) mountpoint = argv[optind++];
	else {
		fprintf(stderr, "No mountpoint and path to archive provided\n");
		return WRONG_PARAMS;
	}

	if (optind < argc) archive_path = argv[optind++];
	else {
		fprintf(stderr, "No path to archive provided\n");
		return WRONG_PARAMS;
	}

	archive_path = realpath(archive_path, NULL);
	if (archive_path == NULL) {
		perror("Error solving the path to the archive");
		return WRONG_PARAMS;
	}

	struct stat st = {0};
	if (stat(mountpoint, &st) == -1) {
	    mkdir(mountpoint, 0755);
	}
	mountpoint = realpath(mountpoint, NULL);
	if (archive_path == NULL) {
		perror("Error solving the mountpoint");
		return WRONG_PARAMS;
	}

	if (optind != argc) {
		fprintf(stderr, "Too many args provided, make sure you entered parameters correctly\n");
		return WRONG_PARAMS;
	}

	if ((getuid() == 0) || (geteuid() == 0)) {
    printf("Running fuse with root privileges, you may consider security implications as any user has unlimited acces to the files inside\n");
	}

	// Now try to mount the archive
	int ret = mount_archive(mountpoint, archive_path);

	// Free used resources
	free(archive_path);

	return 0;
}
