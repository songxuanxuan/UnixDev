#ifndef _DGRAM_H
#define _DGRAM_H

#define oops(m) do{perror(m);exit(1);}while(0)

int make_dgram_server_socket(int port);
int get_internet_address(char*, int, int*, struct sockaddr_in*);
void say_who_called(struct sockaddr_in*);
int make_dgram_client_socket();
int make_internet_address(char* hostname, int port, struct sockaddr_in* ap);

#endif // _DGRAM_H__