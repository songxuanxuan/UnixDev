#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <utmp.h>

#define oops(s,n) {perror(s);exit(n);}
#define DEV_PATH "/dev/"
#define	NAME_SIZE 32

int find_uline_name(char* uline,const char* name);

int main(int ac, char* av[]) {

#if 1
	if (ac < 2)
		oops("param too less", 1);
#endif

	int fd;
	char buf[BUFSIZ];
	char pts_name[NAME_SIZE];
	char uline[NAME_SIZE];
	strcat(pts_name, DEV_PATH);
	if (find_uline_name(uline, av[1]) == -1)
		oops("error finding user", 2);
	strcat(pts_name, uline);
	printf("----%s\n", uline);
	printf("----%s\n", pts_name);
	if ((fd = open(pts_name, O_WRONLY)) == -1)
		oops("open failure", 3);
	while (fgets(buf, BUFSIZ, stdin) != NULL)
	{
		if (write(fd, buf, strlen(buf)) == -1) {
			perror("write failure");
			break;
		}

	}
	close(fd);
	
}

int find_uline_name(char* uline,const char* name)
{
	struct utmp ubuf;
	int fd;
	if ((fd = open(UTMP_FILE,O_RDONLY)) == -1)
		oops("open utmp failure",1);
	int ret;
	while ((ret = read(fd, &ubuf, sizeof(ubuf))) != -1 && ret != 0) {
		if (strcmp(name, ubuf.ut_user) == 0) {
			
			strcpy(uline, ubuf.ut_line);
			/*if(setreuid(0, 0) == -1)
				oops("setreuid",1);*/
			return 0;
		}
	}
	return -1;

}

