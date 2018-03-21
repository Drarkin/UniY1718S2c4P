#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>


#define buffersize 1024
#define myRecv myrecv
extern char myBuffer [buffersize];
extern struct sockaddr_in LastInAddr;
int mySend(char *msg,int fd, struct sockaddr_in addr);
int mysend(int fd, struct sockaddr_in addr);
int myrecv(int fd,struct sockaddr_in addr);