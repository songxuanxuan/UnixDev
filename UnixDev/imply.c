#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#define SLEEPTIME 2
#define BEEP() putchar('\a')

#define oops(s,x) {perror(s);exit(x);}

void test_ecostate() {
	struct termios info;
	int rv;

	rv = tcgetattr(0, &info);	//read value from driver (0 is std input fd)
	if (info.c_lflag & ECHO)
		printf("echo is on \n");
	else
		printf("echo is off \n");
}

void test_setecho(char is) {
	struct termios info;

	if (tcgetattr(0, &info) == -1)
		oops("tcgetattr", 1);

	if (is == 'y')
		info.c_lflag |= ECHO;
	else
		info.c_lflag &= ~ECHO;
	if (tcsetattr(0, TCSANOW, &info) == -1)
		oops("tcsetattr", 2);
}


void test_screen_dimensions() {
	struct winsize wbuf;
	if (ioctl(0, TIOCGWINSZ, &wbuf) != -1) {
		printf("%d row , %d cols", wbuf.ws_row, wbuf.ws_col);
		printf("%d wide , %d tall", wbuf.ws_xpixel, wbuf.ws_ypixel);
	}
}

//设置标准输出模式,设置~ICANON,~ECHO

int get_ok_char();
void tty_mode(int how);
void set_chrmode();
void set_nodelay_mode();
void ctrl_c_handler(int);
int test_tty_mode(int tries) {
	int input;
	printf("continue? (y/n)");
	fflush(stdout);
	tty_mode(0);
	set_chrmode();
	set_nodelay_mode();
	signal(SIGINT, ctrl_c_handler);
	//signal(SIGQUIT, SIG_IGN);		//ctrl-\ SIGQUIT
	while (1) {
		//sleep(SLEEPTIME);
		input = tolower(get_ok_char());
		if(input =='y')
			break;
		if (input == 'n')
			break;
		if (tries-- == 0)
			break;
		BEEP();
	}
	tty_mode(1);
}

//Unix有两个信号是不能被忽略和捕捉的，这两个信号是SIGSTOP(Crtl+Z)和SIGKILL
void ctrl_c_handler(int signum){
	printf("ctrl-c pressed\n");
	tty_mode(1);
	exit(1);
}


int get_ok_char() {
	int c = 'y';
	while ((c = getchar()) != EOF && strchr("yYnN", c) == NULL)
		;
	return c;
}

void tty_mode(int how) {
	static struct termios origin;
	static int origin_flags;
	static int made = 0;
	if (how == 0) {
		tcgetattr(0, &origin);
		origin_flags = fcntl(0, F_GETFL);
		made = 1;
	}
	else if(made){
		tcsetattr(0, TCSANOW, &origin);
		fcntl(0, F_SETFL, origin_flags);
	}
}

void set_chrmode() {
	struct termios ttystate;
	tcgetattr(0, &ttystate);
	ttystate.c_lflag &= ~ICANON;
	ttystate.c_lflag &= ~ECHO;
	ttystate.c_cc[VMIN] = 1;
	//ttystate.c_cc[VTIME] = 20;	//这个应该设置的是延迟
	tcsetattr(0, TCSANOW, &ttystate);
}


//notes: tcsetattr() will do something similar , but it is complicated
void set_nodelay_mode() {
	int termflag;
	termflag = fcntl(0, F_GETFL);
	termflag |= O_NDELAY;
	fcntl(0, F_SETFL, termflag);

}