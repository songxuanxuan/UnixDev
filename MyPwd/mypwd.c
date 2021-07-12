#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

ino_t get_inode(char*);
void printpathto(ino_t);
void inum_to_name(ino_t, char*, int);

int main() {
	printpathto(get_inode("."));
	putchar('\n');
	return 0;
}

void printpathto(ino_t this_node) {
	ino_t my_node;
	char its_name[BUFSIZ];
	if (get_inode("..") != this_node) {
		chdir("..");
		inum_to_name(this_node, its_name, BUFSIZ);
		my_node = get_inode(".");
		printpathto(my_node);
		printf("/%s", its_name);
	}
}

void inum_to_name(ino_t inode, char* namebuf, int buflen)
{
	DIR* dir_ptr;
	struct dirent* direntp;
	dir_ptr = opendir(".");
	if (dir_ptr == NULL) {
		perror(".");
		exit(1);
	}
	while((direntp = readdir(dir_ptr)) != NULL)
		if (direntp->d_ino == inode) {
			strncpy(namebuf, direntp->d_name, buflen);
			closedir(dir_ptr);
			return;
		}
	fprintf(stderr, "error looking for inum %d\n", inode);
	exit(1);
}


ino_t get_inode(char* fname){
	struct stat info;
	if (stat(fname, &info) == -1) {
		fprintf(stderr, "cannot stat ");
		perror(fname);
		exit(1);
	}
	return info.st_ino;
}

