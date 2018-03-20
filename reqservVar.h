#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <string.h>
#include <stdlib.h>

#define IPSIZE 16
#define myMaxTCP 10
#define STDIN 0

#define max(A,B) ((A)>=(B)?(A):(B))
#define myScmp(A) (strncmp(myBuffer,A,strlen(A))==0)
//configuration of App

//Geral Default Values
extern	char	DV_csName[];
extern 	int		DV_cspt;
extern	struct hostent *csh;
extern	struct in_addr *csa;
//Config Params [CentralServer]
extern 	char 			csip[IPSIZE];	// ipv4 string max size
extern 	unsigned int 	cspt;			//port
extern	struct 	sockaddr_in SC_addr,S_addr;
extern	int 	ServX;
extern int	Oid;
extern char Oip[IPSIZE];
extern int	Otpt;
extern	struct reqserv_state {//indicates the system state 
		enum {zero,rs,ts,get,a_get,myON,myOFF} state; //have func getEnum
		enum {off,on} service;// have func getBool
	}state;
//socket
int udp_fp;