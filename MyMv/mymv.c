#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#define FOLDER_PMODE 0755
#define FILE_PMODE 0644
#define NAME_SIZE 256

int check_is_folder(const char* path);

int main(int ac, char* av[]) {
	if (ac != 3) {
		perror("incorrect param");
		exit(1);
	}
	char target[NAME_SIZE];
	int path_len = strlen(av[2]);
	if (av[2][path_len - 1] == '/')
		--path_len;
	memcpy(target, av[2], path_len);
	if (check_is_folder(target)) {
		mkdir(target, FOLDER_PMODE);
		strcat(target, "/");
		strcat(target, av[1]);
	}
	rename(av[1],target);
}

int check_is_folder(const char* path)
{
	struct stat info;
	if (stat(path, &info) == -1) {
		perror(path);
		exit(1);
	}
	return S_ISDIR(info.st_mode);
}

