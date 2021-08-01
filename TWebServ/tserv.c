#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../TinyWeb/servlib.h"
#include "tservlib.h"


#define PORT 8888


int main() {
	pthread_attr_t thread_attr;
	pthread_t worker;
	int sock_fd,client_fd;
	pthread_mutex_t  req_mutex;

	if ((sock_fd = make_server_socket(PORT)) == -1)
		oops("make server");
	//set thread attribute
	set_up(&thread_attr);
	while (1)
	{
		client_fd = accept(sock_fd, NULL, NULL);
		serv_requset_num++;
		int* fdp = malloc(sizeof(int));
		*fdp = client_fd;
		pthread_create(&worker, &thread_attr, handle_call, fdp);
	}

}