#include "varlib.h"

static char* new_string(char*, char*);
static struct var* find_item(char*, int);


int VLstore(char* name, char* val) {
	struct var* itemp;
	char* s;
	int rv = 1;
	if ((itemp = find_item(name, 1)) != NULL && (s = new_string(name, val)) != NULL) {
		if (itemp->str)
			free(itemp->str);
		itemp->str = s;
		rv = 0;
	}
	return rv;
}

char* VLlookup(char* name) {
	struct var* itemp;
	if ((itemp = find_item(name, 0)) != NULL)
		return itemp->str + strlen(name) + 1;
	return " ";
}

int VLexport(char* name) {
	struct var* itemp;
	int rv = 1;

	if ((itemp = find_item(name, 0)) != NULL) {
		itemp->global = 1;
		rv = 0;
	}
	else if (VLstore(name, "") == 1)
		rv = VLexport(name);
	return rv;
}


void VLlist()
{
	for(int i = 0;i<MAXVARS&&tab[i].str != NULL;++i)
	{
		if (tab[i].global)
			printf("* %s\n", tab[i].str);
		else
			printf("%s\n", tab[i].str);
	}
}

char* new_string(char* name, char* val) {
	char* ret = (char*)malloc(strlen(name) + strlen(val) + 2);
	if (ret != NULL)
		sprintf(ret, "%s=%s", name, val);
	return ret;
}

struct var* find_item(char* name, int first_blank)
{
	int len = strlen(name);
	char* s;
	int i;
	for (i = 0; i < MAXVARS && tab[i].str != NULL; ++i) {
		s = tab[i].str;
		//if (strncmp(s, name,len) == 0 && s[len] == '=')
		if (strncmp(s, name,len) == 0)
			return &tab[i];
	}
	if (i < MAXVARS && first_blank)		//如果没有找到,返回尾指针
		return &tab[i];
	return NULL;

}


int VLenviron2table(char* env[]) {
	int i;
	char* newstring;
	for (i = 0; env[i] != NULL; ++i) {
		if (i == MAXVARS)
			return 0;
		if ((newstring = (char*)malloc(strlen(env[i]) + 1)) == NULL) {
			perror("env malloc");
			return 0;
		}
			
		strcpy(newstring, env[i]);
		tab[i].str = newstring;
		tab[i].global = 1;
	}
	while (i < MAXVARS) {
		tab[i].str = NULL;
		tab[i++].global = 0;
	}
	return 1;
}

char** VLtable2environ() {
	int i, j, n = 0;
	char** env;
	for (i = 0; i < MAXVARS && tab[i].str != NULL; ++i)
		if (tab[i].global == 1)
			++n;
	env = (char**)malloc((n + 1) * sizeof(char*));
	if (env == NULL)
		return NULL;
	for (i = 0,j = 0; i < MAXVARS && tab[i].str != NULL; ++i)
		if (tab[i].global == 1)
			env[j++] = tab[i].str;
	env[j] = NULL;
	return env;
}


