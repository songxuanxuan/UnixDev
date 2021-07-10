#include <stdio.h>
#include <dirent.h>
#include <string.h>

#define MAX_LIST_SIZE 64

void do_ls(char[]);

int main(int ac,char* av[]) {
	if (ac == 1 )
		do_ls(".");
	else
	{
		while (--ac)
		{
			printf("\n %s : \n", *++av);
			do_ls(*av);
		}
	}

}

int cmp(const void* a, const void* b) {
	return (*(int*)a - *(int*)b);
}

void do_ls(char dirname[] )
{
	DIR* dir_ptr;
	struct dirent* direntp;
	char* name_list[MAX_LIST_SIZE];
	if ((dir_ptr = opendir(dirname)) == NULL)
		fprintf(stderr, "ls:cannot open %s \n", dirname);
	else
	{
		int i = 0;
		while ((direntp = readdir(dir_ptr)) != NULL)
		{
			name_list[i++] = direntp->d_name;
		}
		qsort(name_list, i, sizeof(name_list[0]), cmp);
		for (int j = 0; j < i; ++j)
		{
			printf("%s   ", name_list[j]);
			if (j != 0 && j % 9 == 0)
				printf("\n");
		}
		closedir(dir_ptr);

	}
}
