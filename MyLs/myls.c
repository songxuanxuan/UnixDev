#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAX_LIST_SIZE 64

void do_ls(char[],int);
void do_stat(char*);
void show_file_info(char*, struct stat*);
void mode_to_letters(int, char[]);
char* uid_to_name(uid_t);
char* gid_to_name(gid_t);

int main(int ac,char* av[]) {
	int isL = 0;
	if (ac == 1 )
		do_ls(".",isL);
	else if (ac == 2 && strcmp(av[1], "-l") == 0) 
	{
		isL = 1;
		do_ls(".", isL);
	}
	else
	{
		if (strcmp(av[1],"-l") == 0) {
			isL = 1;
			++av;
		}
		while (--ac)
		{
			printf("\n %s : \n", *++av);
			do_ls(*av,isL);
		}
	}

}

int cmp(const void* a, const void* b) {
	return (*(int*)a - *(int*)b);
}

void do_ls(char dirname[] ,int isL)
{
	DIR* dir_ptr;
	struct dirent* direntp;
	char* name_list[MAX_LIST_SIZE];
	if ((dir_ptr = opendir(dirname)) == NULL)
		fprintf(stderr, "ls:cannot open %s \n", dirname);
	else
	{
		if (isL) {
			while ((direntp = readdir(dir_ptr)) != NULL)
			{
				do_stat(direntp->d_name);
			}
		}
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
		}
		closedir(dir_ptr);

	}
}


void do_stat(char* filename)
{
	struct stat info;
	if (stat(filename, &info) == -1)
		perror(filename);		//这里某些文件会出现 找不到的错误 todo
	else
		show_file_info(filename, &info);
}


void show_file_info(char* filename, struct stat* info_p)
{
	char modestr[11];

	mode_to_letters(info_p->st_mode, modestr);

	printf("%s", modestr);
	printf("%4d ", (int)info_p->st_nlink);
	printf("%-8s", uid_to_name(info_p->st_uid));
	printf("%-8s", gid_to_name(info_p->st_gid));
	printf("%8ld", (long)info_p->st_size);
	printf("%.12s ", 4 + ctime(&info_p->st_mtim.tv_sec));
	printf("%s\n", filename);
}

//utility functions

void mode_to_letters(int mode, char str[]) {
	strcpy(str, "--------");
	if (S_ISDIR(mode)) str[0] = 'd';
	if (S_ISCHR(mode)) str[0] = 'c';
	if (S_ISBLK(mode)) str[0] = 'b';

	if (mode & S_IRUSR) str[1] = 'r';
	if (mode & S_IWUSR) str[2] = 'w';
	if (mode & S_IXUSR) str[3] = 'x';

	if (mode & S_IRGRP) str[4] = 'r';
	if (mode & S_IWGRP) str[5] = 'w';
	if (mode & S_IXGRP) str[6] = 'x';

	if (mode & S_IROTH) str[7] = 'r';
	if (mode & S_IWOTH) str[8] = 'w';
	if (mode & S_IXOTH) str[9] = 'x';
}

char* uid_to_name(uid_t uid) {
	struct passwd* pw_ptr;

	static char namestr[10];

	if ((pw_ptr = getpwuid(uid)) == NULL) {
		sprintf(namestr, "%d", uid);
		return namestr;
	}
	else
		return pw_ptr->pw_name;
}

char* gid_to_name(gid_t gid) {
	struct group* grp_ptr;
	static char grpstr[10];

	if ((grp_ptr = getgrgid(gid)) == NULL) {
		sprintf(grpstr, "%s", gid);
		return grpstr;
	}
	else
		return grp_ptr->gr_name;
}




