#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

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


//#ifdef app_service
	//Default Values
extern	int	DV_spt;
	//Config Params
extern	unsigned int	id;
extern	char			*ip;
extern	unsigned int	upt;
extern	unsigned int	tpt;
	//socket
extern	int 	udp_fp;
extern	int 	tcp_fd;
extern	struct 	sockaddr_in sudp,stcp;
	//SC adress
extern	struct 	sockaddr_in SC_addr;	
	//SC NextRingServer
extern	struct 	sockaddr_in ring_addr;//tcp addres to comunivcate with other ring elements
	//Other ServiceServerInfo
extern	struct app_service_state {//indicates the system state 
		enum {nready,s_ds,s_ds_ok,w_ds,w_ds_ok,s_s,s_s_ok,w_s,w_s_ok,g_s,g_s_ok,ready,busy} state; //have func getEnum
		enum {false,true} ds,ss,ring;// have func getBool
	}AppState;
extern	int 	ServX;
extern	int		Oid;
extern	char	Oip[IPSIZE];
extern	int		Otpt;
//#endif