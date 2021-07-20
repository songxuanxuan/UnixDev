#include <stdio.h>
#include <string.h>
#include <stdlib.h>
enum {
	NEUTRAL,
	WANT_THEN,
	THEN_BLOCK,
	WANT_ELSE,
	ELSE_BLOCK
};
enum {
	SUCCESS,
	FAIL
};

static int if_state = NEUTRAL;
static int if_result = SUCCESS;
static int last_state = 0;

int process(char** args);

int syn_err(char* msg) {
	if_state = NEUTRAL;
	fprintf(stderr, "syntax error: %s \n", msg);
	return -1;
}

int ok_to_execute() {
	int ret = 1;
	if (if_state == WANT_THEN) {
		syn_err("then expected");
		ret = 0;
	}
	else if (if_state == THEN_BLOCK && if_result == FAIL)
		ret = 0;
	else if (if_state == ELSE_BLOCK && if_result == SUCCESS)
		ret = 0;
	return ret;
}

int is_control_command(char* s) {

	return (strcmp(s, "if") == 0 || strcmp(s, "then") == 0 
		|| strcmp(s, "fi") == 0 || strcmp(s, "else") == 0);
}

int do_control_command(char** args) {
	char* cmd = args[0];
	int ret = -1;
	if (strcmp(cmd, "if") == 0)
		if (if_state != NEUTRAL)
			ret = syn_err("if unexpected");
		else {
			last_state = process(args + 1);
			if_result = (last_state == 0 ? SUCCESS : FAIL);
			if (if_result == SUCCESS)
				if_state = WANT_THEN;
			else
				if_state = WANT_ELSE;
			ret = 0;
		}
	else if (strcmp(cmd, "then") == 0 )
		if (if_state != WANT_THEN && if_state != WANT_ELSE)
			ret = syn_err("then unexpected");
		else {
			if_state = THEN_BLOCK;
			ret = 0;
		}
	else if (strcmp(cmd, "fi") == 0)
		if (if_state != THEN_BLOCK && if_state != ELSE_BLOCK )
			ret = syn_err("fi unexpected");
		else {
			if_state = NEUTRAL;
			ret = 0;
		}
	else if (strcmp(cmd, "else") == 0)
		if (if_state != THEN_BLOCK)
			ret = syn_err("else unexpected");
		else {
			if_state = ELSE_BLOCK;
			ret = 0;
		}
	else
	{
		perror("internal error");
		exit(1);
	}
	return ret;
}