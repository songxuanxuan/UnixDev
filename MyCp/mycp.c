
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_SIZE 16
#define COPY_MODE 0644


void oops(const char* s1, const char* s2);
int main(int ac, char* av[]) {
	int in_fd, out_fd, n_chars;
	int n;
	char buf[BUFFER_SIZE];
	if (ac != 3) {
		fprintf(stderr, "error usage: %s source_distination \n", *av);
		exit(1);
	}
	if (strcmp(av[1], av[2]) == 0) {
		fprintf(stderr,"mycp: %s and %s are the same file",av[1],av[2]);
		exit(1);
	}
	if ((in_fd = open(av[1], O_RDONLY)) == -1)
		oops("cannot open", av[1]);
	if ((out_fd = creat(av[2], COPY_MODE)) == -1)
		oops("cannot create", av[2]);
	while ((n_chars = read(in_fd, buf, BUFFER_SIZE)) > 0) {
		if ((n = write(out_fd, buf, n_chars)) != n_chars)
			oops("write error to", av[2]);
	}

	if (n_chars == -1)
		oops("read error from ", av[1]);
	if (close(out_fd) == -1 || close(in_fd) == -1)
		oops("closing error file", " ");

}

void oops(const char* s1, const char* s2) {
	fprintf(stderr, "Error: %s ", s1);
	perror(s2);
	exit(1);
}