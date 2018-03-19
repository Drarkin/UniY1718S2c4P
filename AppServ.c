/*
Made by Drarkin (jefc)
*/

#include "AppServSystem.h"

#define app_service

int main (int argc, char **argv) {
	appConfg(argc,argv);
	#ifdef app_service
		//udp SC ask for SA
			//SetUp Sockets
			udp_fp=socket(AF_INET,SOCK_DGRAM,0);//UDP socket
			if(udp_fp==-1)myerr(2,"SocketFail Err001");//error
			memset((void*)&SC_addr,(int)'\0',sizeof(SC_addr));
				SC_addr.sin_family=AF_INET;
				SC_addr.sin_addr=*csa;//MachineIP
				SC_addr.sin_port=htons(cspt);
			
			memset((void*)&sudp,(int)'\0',sizeof(sudp));
				sudp.sin_family=AF_INET;
				sudp.sin_addr.s_addr=htonl(INADDR_ANY);//MachineIP
				sudp.sin_port=htons(upt);
			if(bind(udp_fp,(struct sockaddr*)&sudp,sizeof(struct sockaddr))==-1)myerr(3,"FAiled to bind udp");
				
			
			tcp_fd=socket(AF_INET,SOCK_STREAM,0);
			if(tcp_fd==-1)myerr(2,"SocketFail Err002");//error
			memset((void*)&stcp,(int)'\0',sizeof(stcp));
				stcp.sin_family=AF_INET;
				stcp.sin_addr.s_addr=htonl(INADDR_ANY);//MachineIP
				stcp.sin_port=htons(tpt);
			if(bind(tcp_fd,(struct sockaddr*)&stcp,sizeof(struct sockaddr))==-1)myerr(3,"FAiled to bind tcp");
			if(listen(tcp_fd,myMaxTCP)==-1)myerr(4,"Fail to set maxConnects");
			appRun();
			myfpClose();
	#endif
	exit(0);
}
