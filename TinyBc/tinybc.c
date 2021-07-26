#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define oops(s) do{perror(s);exit(1);}while(0)

#define BUFSIZ 128

void be_dc(int in[2], int out[2]) {
	if (dup2(in[0], 0) == -1)
		oops("dup2");
	close(in[1]);
	close(in[0]);
	if (dup2(out[1], 1) == -1)
		oops("dup2");
	close(out[1]);
	close(out[0]);

	execlp("dc", "dc","-", NULL);	//- option
	oops("execlp");
}

void be_bc(int todc[2], int fromdc[2]) {
	char operator[BUFSIZ], msg[BUFSIZ];
	int left, right;
	close(todc[0]);
	close(fromdc[1]);
	FILE* fpout, * fpin;
	fpout = fdopen(todc[1], "w");
	fpin = fdopen(fromdc[0], "r");
	if (fpout == NULL || fpin == NULL)
		oops("convering pipes to stream");
	while (printf("tinybc: "), fgets(msg, BUFSIZ, stdin) != NULL)
	{
		if (msg[0] == 'q') break;
		if (sscanf(msg, "%d%[+-*/^]%d", &left, operator,&right) != 3)
		{
			perror("scanf syntax");
			continue;
		}
		if (fprintf(fpout, "%d\n%d\n%c\np\n", left, right, *operator) == EOF)
			oops("fpout write");
		fflush(fpout);
		if (fgets(msg, BUFSIZ, fpin) == NULL)
			break;
		printf("%d %c %d = %s", left, *operator,right, msg);
	}
	fclose(fpin);
	fclose(fpout);
}

int main() {
	int pid, todc[2], fromdc[2];
	if (pipe(todc) == -1 || pipe(fromdc) == -1)
		oops("pipe");
	pid = fork();
	if (pid == -1)
		oops("fork");
	else if (pid == 0)
		be_dc(todc, fromdc);
	else {
		be_bc(todc, fromdc);
		wait(NULL);  //wait for close childp ?
	}
}