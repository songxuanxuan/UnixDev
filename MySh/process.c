#include <stdio.h>
int ok_to_execute();
int is_control_command(char* s);
int do_control_command(char** args);
int execute(char**);


int process(char** args) {
	int ret = 0;
	if (args[0] == NULL)
		ret = 0;
	else if (is_control_command(*args))
		ret = do_control_command(args);
	else if (ok_to_execute())
		ret = execute(args);
	return ret;
}