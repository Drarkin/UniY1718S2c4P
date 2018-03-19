#include "AppServVar.h"

//configuration of App

//Geral Default Values
char	DV_csName[]="tejo.tecnico.ulisboa.pt";
int	DV_cspt=59000;


struct hostent *csh;
struct in_addr *csa;

//Config Params [CentralServer]
char 		csip[IPSIZE];	// ipv4 string max size
unsigned int 	cspt;	//port


//#ifdef app_service
	//Default Values
	int	DV_spt=59000;
	//Config Params
	unsigned int	id;
	char		*ip;
	unsigned int	upt;
	unsigned int	tpt;
	//socket
	int udp_fp=-1;
	int tcp_fd=-1;
	struct sockaddr_in sudp,stcp;
	//SC adress
	struct sockaddr_in SC_addr;	
	//SC NextRingServer
	struct sockaddr_in ring_addr;//tcp addres to comunivcate with other ring elements
	//Other ServiceServerInfo
	struct app_service_state AppState;
	int ServX;
	int	Oid;
	char	Oip[IPSIZE];
	int	Otpt;
//#endif