#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "varlib.h"


	 
int assign(char*);
int okname(char*);

int builtin_command(char** args, int* resultp) {
	int rv = 0;
	if (strcmp(args[0], "set") == 0) {
		VLlist();
		*resultp = 0;
		rv = 1;
	}
	else if (strchr(args[0], '=') != NULL) {
		*resultp = assign(args[0]);
		if (*resultp != -1)
			rv = 1;
	}
	else if (strcmp(args[0], "export") == 0) {
		if (args[1] != NULL && okname(args[1]))
			*resultp = VLexport(args[1]);
		else
			*resultp = 1;
		rv = 1;
	}
	return rv;

}

int assign(char* str) {
	char* cp = strchr(str, '=');
	*cp = '\0';
	int rv = okname(str) ? VLstore(str, cp + 1) : -1;
	*cp = '=';
	return rv;
}

int okname(char* str) {
	char* cp = str;
	while (*cp)
	{
		if ((isdigit(*cp) && cp == str) || !(isalnum(*cp) || *cp == '_'))
			return 0;
		++cp;
	}
	return (cp != str);		//如果是空返回false
}
