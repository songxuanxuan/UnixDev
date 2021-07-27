#ifndef _SERVLIB_H
#define _SERVLIB_H

//make server bind,return socket fd;
int make_server_socket(int portnum);

//connect to host:portnum ,return socket fd;
int connect_to_server(char* host, int portnum);

//for SIGCHLD signal action
void child_wait(int signum);

#endif // _SERVLIB_H__