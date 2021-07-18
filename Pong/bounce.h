#ifndef _BOUNCE_H
#define _BOUNCE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <curses.h>
#include <string.h>
#include <aio.h>

#define LEFT_EDGE 10
#define RIGHT_EDGE 70
#define TOP_ROW 5
#define BOT_ROW 20
#define X_INIT 8
#define Y_INIT 14
#define TICKS_PER_SEC 100

#define X_TTM 32
#define Y_TTM 32
#define MESSAGE "HELLO"
#define D_SIMBOL 'o'
#define BLANK_CH ' '
#define BLANK   "     "

#define X_PLATE (RIGHT_EDGE / 2)
#define Y_PLATE BOT_ROW
#define PLATE_SYMBOL "-----"
#define PLATE_DIR 5


struct ppball {
	int y_pos, x_pos,
		y_ttm, x_ttm,
		y_ttg, x_ttg,
		y_dir, x_dir;

	char symbol;
};

struct plate {
	int x, y;
	int x_dir;
	char p_symbol[10];
};

#endif // _BOUNCE_H__
