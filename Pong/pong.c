#include "bounce.h"

int row = TOP_ROW;
int col;
int dir;

void move_msg(int);
void bounce1d();

void countdown(int);
int set_ticker(int);
void int_handler(int signum);
void test_sigaction();


struct ppball the_ball;
struct plate the_plate;
struct aiocb keyboard_cb;

void ball_move(int signum);
void set_up();
void move_plate(int is_left);
void break_out() {
	set_ticker(0);
	endwin();
	exit(0);
}

int main() {
	int num = 0;
	set_up();
	while (1) {
		mvaddch(BOT_ROW, RIGHT_EDGE, ++num + '0');
		refresh();
		int time = 0;
		do 
		{
			time = sleep(5);
		} while (time > 0);
		
	}
	break_out();
}


//move 2D


#define MV_PALTE(symbol) mvaddstr(the_plate.y, the_plate.x, symbol)

int is_touched(struct plate* p_plate, struct ppball* p_ball) {
	if(p_ball->y_pos == p_plate->y 
		&& p_ball->x_pos >= p_plate->x 
		&& p_ball->x_pos <= p_plate->x + strlen(p_plate->p_symbol))
		return 0;
	else
		return -1;
}

void move_plate(int is_left) {
	if (is_left && the_plate.x >= LEFT_EDGE) {
		MV_PALTE(BLANK);
		the_plate.x -= the_plate.x_dir;
		MV_PALTE(the_plate.p_symbol);
		refresh();
	}
	else if(!is_left && the_plate.x <= RIGHT_EDGE)
	{
		MV_PALTE(BLANK);
		the_plate.x += the_plate.x_dir;
		MV_PALTE(the_plate.p_symbol);
		refresh();
	}
}
int bounce_or_lose(struct ppball* bp) {
	int ret = 0;
	if (bp->y_pos <= TOP_ROW) {
		bp->y_dir = 1;
		ret = 1;
	}
	else if (bp->y_pos >= BOT_ROW) {
		if (is_touched(&the_plate, bp) == -1)
			return -1;
		bp->y_dir = -1;
		ret = 1;
	}

	if (bp->x_pos <= LEFT_EDGE) {
		bp->x_dir = 1;
		ret = 1;
	}
	else if (bp->x_pos >= RIGHT_EDGE) {
		bp->x_dir = -1;
		ret = 1;
	}
	return ret;

}

void ball_move(int signum)
{
	int y_cur, x_cur,moved;
	signal(SIGALRM, SIG_IGN);
	y_cur = the_ball.y_pos;
	x_cur = the_ball.x_pos;

	if (the_ball.y_ttm > 0 && the_ball.y_ttg-- == 1) {
		the_ball.y_pos += the_ball.y_dir;
		the_ball.y_ttg = the_ball.y_ttm;
		moved = 1;
	}
	if (the_ball.x_ttm > 0 && the_ball.x_ttg-- == 1) {
		the_ball.x_pos += the_ball.x_dir;
		the_ball.x_ttg = the_ball.x_ttm;
		moved = 1;
	}
	if (moved) {
		mvaddch(y_cur, x_cur, BLANK_CH);
		mvaddch(the_ball.y_pos, the_ball.x_pos, the_ball.symbol);
		if (bounce_or_lose(&the_ball) == -1)
			break_out();
		move(LINES - 1, COLS - 1);
		refresh();
	}
	signal(SIGALRM, ball_move);

}

void on_input(int signum) {
	int c;
	int* cp = (int *)keyboard_cb.aio_buf;

	if (aio_error(&keyboard_cb) != 0)
		perror("reading aio");
	else
		if (aio_return(&keyboard_cb) == 1) {
			c = *cp;
			switch (c)
			{
			case 'p':
				set_ticker(0);
			case 'f':
				--the_ball.x_ttm;
				--the_ball.y_ttm;
				break;
			case 's':
				++the_ball.x_ttm;
				++the_ball.y_ttm;
				break;
			case 'a':	//todo: aio 中如何获取方向键
				move_plate(1);
				break;
			case 'd':
				move_plate(0);
				break;
			default:
				break;
			}
			
		}
	aio_read(&keyboard_cb);
}
void set_aio_buffer() {
	static int input[1];

	keyboard_cb.aio_fildes = 0;
	keyboard_cb.aio_buf = input;
	keyboard_cb.aio_nbytes = 4;
	keyboard_cb.aio_offset = 0;

	keyboard_cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	keyboard_cb.aio_sigevent.sigev_signo = SIGIO;

}

void set_up() {
	
	the_ball.x_pos = X_INIT;
	the_ball.y_pos = Y_INIT;
	the_ball.y_ttm = the_ball.y_ttg =Y_TTM;
	the_ball.x_ttm = the_ball.x_ttg =X_TTM;
	the_ball.symbol = D_SIMBOL;
	the_ball.x_dir = the_ball.y_dir = 1;

	the_plate.x = X_PLATE;
	the_plate.y = Y_PLATE;
	the_plate.x_dir = PLATE_DIR;
	strcpy(the_plate.p_symbol, PLATE_SYMBOL);

	initscr();
	noecho();
	crmode();	//进入cbreak模式
	keypad(stdscr,TRUE);	//todo:在 aio 下失效

	signal(SIGALRM, SIG_IGN);
	mvaddch(the_ball.y_pos, the_ball.x_pos, the_ball.symbol);
	mvaddstr(the_plate.y, the_plate.x, the_plate.p_symbol);
	refresh();

	signal(SIGIO, on_input);
	set_aio_buffer();
	aio_read(&keyboard_cb);	//place in request queue


	signal(SIGALRM, ball_move);
	set_ticker(1000 / TICKS_PER_SEC);
}




//move 1D

void bounce1d() {
	initscr();
	crmode();	//进入cbreak模式
	noecho();
	clear();

	int delay = 200;
	int ndelay;
	char c;
	col = 0;
	dir = 1;
	move(row, col);
	addstr(MESSAGE);
	signal(SIGALRM, move_msg);
	set_ticker(delay);
	while (1) {
		ndelay = 0;
		c = getch();	//input without buffer
		if (c == 'q') break;
		if (c == ' ') dir = -dir;
		if (c == 'f' && delay > 2) ndelay = delay / 2;
		if (c == 's') ndelay = delay * 2;
		if (ndelay > 0)
			set_ticker(ndelay);
	}
	endwin();


}

void move_msg(int signum)
{
	
	
	signal(SIGALRM, SIG_IGN);	//reset just in case
	move(row, col);
	addstr(BLANK);
	col += dir;
	move(row, col);
	addstr(MESSAGE);
	refresh();
	if (dir == -1 && col <= 0)
		dir = 1;
	else if (dir == 1 && col + strlen(MESSAGE) >= COLS)
		dir = -1;
	signal(SIGALRM, move_msg);
}




void test_sigaction() {
	struct sigaction newhandler;
	sigset_t blocked;
	char in[128];

	newhandler.sa_handler = int_handler;
	newhandler.sa_flags = SA_RESETHAND | SA_RESTART;

	sigemptyset(&blocked);
	sigaddset(&blocked, SIGQUIT);
	newhandler.sa_mask = blocked;

	if (sigaction(SIGINT, &newhandler, NULL) == -1)
		perror("sigaction");
	else
		while (1) {
			fgets(in, sizeof(in), stdin);
			if (in[0] == 'q')
				kill(0, SIGINT);
			/*if (in[0] == '\\')
				kill(getpid(), SIGQUIT);*/
			printf("input : %s", in);
		}

}

void test_ALRM() {
	signal(SIGALRM, int_handler);
	if (set_ticker(500) == -1)
		perror("set_ticker");
	else
		while (1) {
			pause();
		}
	
}

void int_handler(int signum) {
	printf("called %d \n",signum);
	fflush(stdout);
	sleep(signum);
}


void countdown(int signum){
	static int num = 10;
	printf("%d..", num--);
	fflush(stdout);
	if (num < 0) {
		printf("DONE! \n");
		exit(0);
	}
}


int set_ticker(int n_msecs) {
	struct itimerval new_timeset;
	long n_sec, n_microsec;
	n_sec = n_msecs / 1000;
	n_microsec = (n_msecs % 1000) * 1000L;	//Microseconds

	new_timeset.it_interval.tv_sec = n_sec;
	new_timeset.it_interval.tv_usec = n_microsec;

	new_timeset.it_value.tv_sec = n_sec;
	new_timeset.it_value.tv_usec = n_microsec;

	return setitimer(ITIMER_REAL, &new_timeset, NULL);

}

