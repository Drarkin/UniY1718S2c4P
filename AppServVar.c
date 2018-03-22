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
//Client adress	
struct sockaddr_in C_addr;
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
const char *AppMainStateName[]={"nready", "g_s", "g_s_ok", "s_s", "s_s_ok", "s_ds", "s_ds_ok", "joinR", "joinR_ok", "leaveR", "leaveR_ok", "ready", "busy", "w_ds", "w_ds_ok", "w_s", "w_s_ok"};
/*const char *getstate(){
	return AppMainStateName[AppState.state];
	/*switch(AppState.state){
		case nready:return "nready";
		case g_s:return "g_s";
		case g_s_ok:return "g_s_ok";
		case s_s:return "s_s";
		case s_s_ok:return "s_s_ok";
		case s_ds:return "s_ds";
		case s_ds_ok:return "s_ds_ok";
		case joinR:return "joinR";
		case joinR_ok: return "joinR_ok";
		case leaveR: return "leaveR";
		case leaveR_ok: return "leaveR_ok";
		case ready: return "ready";
		case busy: return "busy";
		case w_ds: return "w_ds";
		case w_ds_ok: return "w_ds_ok";
		case w_s: return "w_s";
		case w_s_ok:return "w_s_ok";
		default: return "Unknown";
	}
}*/