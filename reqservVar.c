#include "reqservVar.h"

//configuration of App

//Geral Default Values
char	DV_csName[]="tejo.tecnico.ulisboa.pt";
int	DV_cspt=59000;


struct hostent *csh;
struct in_addr *csa;

//Config Params [CentralServer]
char 		csip[IPSIZE];	// ipv4 string max size
unsigned int 	cspt;	//port
//SC adress
struct sockaddr_in SC_addr, S_addr;	
int ServX;
int	Oid;
char Oip[IPSIZE];
int	Otpt;
struct reqserv_state state;
//saocket
int udp_fp=-1;