#include <cstdio>
#include <cstdlib>


#define PAGE_LEN 24
#define LINE_LINE 512
void do_more(FILE*);
int see_more(FILE*);

int main(int ac,char* av[]) {
	FILE* fp;
	if (ac == 1)
		do_more(stdin);
	else
		while (--ac)
			if ((fp = fopen(*++av, "r")) != NULL) {
				do_more(fp);
				fclose(fp);
			}
			else
				exit(1);
	return 0;
}

void do_more(FILE* fp) {
	char line[LINE_LINE];
	int num_of_line = 0;
	int reply;
	FILE* fp_tty = fopen("/dev/tty", "r");
	if (fp_tty == NULL)
		exit(1);
	while (fgets(line,LINE_LINE,fp))
	{
		if (num_of_line == PAGE_LEN) {
			reply = see_more(fp_tty);
			if(reply == 0)
				break;
			num_of_line -= reply;
		}
		if (fputs(line, stdout) == EOF)
			exit(1);
		num_of_line++;
	}
}

int see_more(FILE* cmd) {
	int c;
	printf(" more? ");
	while ((c = getc(cmd))!=EOF)
	{
		if (c == 'q')
			return 0;
		if (c == ' ')
			return PAGE_LEN;
		if (c == '\n')
			return 1;
	}
	return 0;
}