#define _GNU_SOURCE

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_PATH 256

#define ERROR -1
#define OK 0

bool should_ignore(char *file_name);

DIR *get_subdir(DIR *parent_dir, char *dir_name);

void find_rec(DIR *dir,
              char *match,
              char *path_to_dir,
              char *locate_substring(const char *haystack, const char *needle));

void find(char *match,
          char *locate_substring(const char *haystack, const char *needle));


int
main(int argc, char *argv[])
{
	switch (argc) {
	case 2:
		find(argv[1], strstr);
		break;

	case 3:
		if (strcmp(argv[1], "-i") != 0) {
			printf("Argumentos invalidos");
			exit(ERROR);
		}
		find(argv[2], strcasestr);
		break;
	}


	return OK;
}

void
find(char *match, char *locate_substring(const char *haystack, const char *needle))
{
	DIR *dir = opendir(".");
	find_rec(dir, match, ".", locate_substring);
}

void
find_rec(DIR *dir,
         char *match,
         char *path_to_dir,
         char *locate_substring(const char *haystack, const char *needle))
{
	struct dirent *entry = readdir(dir);
	while (entry) {
		if (should_ignore(entry->d_name)) {
			entry = readdir(dir);
			continue;
		}

		if (locate_substring(entry->d_name, match)) {
			printf("%s/%s\n", path_to_dir, entry->d_name);
		}

		if (entry->d_type == DT_DIR) {
			DIR *subdir = get_subdir(dir, entry->d_name);

			char new_prefix[MAX_PATH] = "";
			strcat(new_prefix, path_to_dir);
			strcat(new_prefix, "/");
			strcat(new_prefix, entry->d_name);

			find_rec(subdir, match, new_prefix, locate_substring);
		}

		entry = readdir(dir);
	}
	closedir(dir);
}

bool
should_ignore(char *file_name)
{
	return strcmp(file_name, "..") == 0 || strcmp(file_name, ".") == 0;
}

DIR *
get_subdir(DIR *parent_dir, char *dir_name)
{
	int parent_fd = dirfd(parent_dir);
	int sub_fd = openat(parent_fd, dir_name, O_RDONLY);
	DIR *subdir = fdopendir(sub_fd);
	return subdir;
}
