#ifndef _TSERVLIB_H
#define _TSERVLIB_H


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define oops(x) do{perror(x);return;}while (0)

static int serv_requset_num;
static int serv_bytes_num;

//set up thread to avoid zombie thread 
void set_up(pthread_attr_t * attrp);
void* handle_call(void* fdp);

#endif // _TSERVLIB_H__
