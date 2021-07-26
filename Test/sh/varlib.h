#ifndef _VARLIB_H
#define _VARLIB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXVARS 128

struct var {
	char* str;
	int global;
};
void VLlist();
int VLexport(char*);
int VLstore(char*, char*);
int VLenviron2table(char* env[]);
char** VLtable2environ();


static struct var tab[MAXVARS];

#endif // _VARLIB_H__
