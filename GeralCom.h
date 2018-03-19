#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>

#define buffersize 1024
extern char myBuffer [buffersize];
int mysend(int fp, struct sockaddr_in addr);
int myrecv(int fp,struct sockaddr_in addr);