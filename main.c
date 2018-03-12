/*
Made by Drarkin (jefc)
*/

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define app_service
#define debugInfo
#define buffersize 1024
#define IPSIZE 16
#define myMaxTCP 10

void myfpClose();
void myerr(int code,char* msg){
	fprintf(stderr,"Err_%d",code);
	if (msg!=NULL) fprintf(stderr,":%s",msg);
	fprintf(stderr,"\n");
	#ifdef app_service
		myfpClose();
	#endif
	exit(-code);
	}

char myBuffer [buffersize];

//configuration of App

//Geral Default Values
char	DV_csName[]="tejo.tecnico.ulisboa.pt";
int	DV_cspt=59000;


struct hostent *csh;
struct in_addr *csa;

//Config Params [CentralServer]
char 		csip[IPSIZE];	// ipv4 string max size
unsigned int 	cspt;	//port

#ifdef app_service
	//Default Values
	int	DV_spt=59000;
	//Config Params
	unsigned int	id;
	char		*ip;
	unsigned int	upt;
	unsigned int	tpt;
	//socket
	int udp_fp=-1;
	int tcp_fp=-1;
	struct sockaddr_in sudp,stcp;
	//SC adress
	struct sockaddr_in SC_addr;	
	//Other ServiceServerInfo
	int	Oid;
	char	Oip[IPSIZE];
	int	Otpt;
#endif
#ifndef app_reqserv
	#ifndef app_service
		#error app type not defined
		#include <Error_App_NotDef>
	#endif
#endif

//functions

	void myfpClose(){
		#ifdef app_service
		if (udp_fp>=0){ close(udp_fp);udp_fp=-1;}
		if (tcp_fp>=0){ close(tcp_fp);tcp_fp=-1;}
		#endif
		return;
	}
#ifdef app_service
	void set_ds(int x){
		sprintf(myBuffer,"SET_DS %i;%i;%s;%i\n",x,id,ip,upt);
	}
	void withdraw(int x){
		sprintf(myBuffer,"WITHDRAW %i;%i\n",x,id);
	}
	void set_start (int x){
		sprintf(myBuffer,"SET_START %i;%i;%s;%i\n",x,id,ip,tpt);//verify tpt
	}
	void withdraw_start(int x){
		sprintf(myBuffer,"WITHDRAW_START %i;%i\n",x,id);
	}
	int get_start(int x){
		int n;
		char *mypointer[4];
		int addrlen =(int) sizeof(SC_addr);
		sprintf(myBuffer,"GET_START %i;%i\n",x,id);
		n=sendto(udp_fp,myBuffer,strlen(myBuffer),0,(struct sockaddr*)&SC_addr,sizeof(SC_addr));
		if(n==-1)myerr(2,"Fail to send");
			fprintf(stderr,"Sent: %s\n",myBuffer);
		n=recvfrom(udp_fp,myBuffer,buffersize,0,(struct sockaddr*)&SC_addr,&addrlen);
		if(n==-1)myerr(3,"Fail to recive data");//error
		
		//ProcessAnswer
		fprintf(stdout,"ServerAns: %s\n",myBuffer);
		if (0!=sscanf(myBuffer,"OK %i;%i;%s;%i\n",&n,&Oid,Oip,Otpt) && n==id){
			fprintf(stderr,"AnsData:\n\tid: %i\n\tip %s\n\ttpt %i\n",Oid,Oip,Otpt);
			return 1;
		}
		fprintf(stderr,"Err WrongMsg\n");
		return 0;
	}
#endif

void appConfg ( int argc,char **argv){
	//arg is a vector of string,
	//arg last pointer must be NULL
	int i=0;
	char *SName=DV_csName;//CentralServer Name
	cspt=DV_cspt;
	#ifdef app_service
	int argCheck = 0;
	#endif
	while (argv[i]!=NULL){
		#ifdef app_service
		if (strcmp(argv[i],"-n")==0 && argc>i){
			i++;
			argCheck++;
			id=atoi(argv[i]);
		}else	if (strcmp(argv[i],"-j")==0 && argc>i){
			i++;
			argCheck++;
			ip=argv[i];
		}else   if (strcmp(argv[i],"-u")==0 && argc>i){
                        i++;
			argCheck++;
			upt=atoi(argv[i]);
                }else   if (strcmp(argv[i],"-t")==0 && argc>i){
                        i++;
			argCheck++;
			tpt=atoi(argv[i]);
                }
		#endif
		if (strcmp(argv[i],"-i")==0 && argc>i){
			i++;
			SName=argv[i];
		}else	if (strcmp(argv[i],"-p")==0 && argc>i){
			i++;
			cspt=atoi(argv[i]);
		}
		i++;
	}
	//check arguments for service
	#ifdef app_service
	if(argCheck!=4)myerr(1,"missing arguments");
	#endif
	//get CentralServer IP
	csh = gethostbyname(SName);
	if (csh==NULL) myerr(2,"Fail to gethostbyname");
	csa=(struct in_addr*)csh->h_addr_list[0];
				   
	#ifdef debugInfo
		#ifdef app_service
		fprintf(stderr,"Service:\n\tid: %d\n\tip: %s\n\tupt: %d\n\ttpt: %d\n",id,ip,upt,tpt);
		#endif
	fprintf(stderr,"\nCentralServer:\n\t<%s:%d>\n",SName,cspt);
	fprintf(stderr,"\tofficial host name: %s\n",csh->h_name);
	fprintf(stderr,"\tinternet address: %s (%08lX)\n",inet_ntoa(*csa),(long unsigned int)ntohl(csa->s_addr));
 	#endif
	return;
}
int main (int argc, char **argv) {
	appConfg(argc,argv);
	#ifdef app_service
		//udp SC ask for SA
			//SetUp
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
				
			
			tcp_fp=socket(AF_INET,SOCK_STREAM,0);
			if(tcp_fp==-1)myerr(2,"SocketFail Err002");//error
			memset((void*)&stcp,(int)'\0',sizeof(stcp));
				stcp.sin_family=AF_INET;
				stcp.sin_addr.s_addr=htonl(INADDR_ANY);//MachineIP
				stcp.sin_port=htons(tpt);
			if(bind(tcp_fp,(struct sockaddr*)&stcp,sizeof(struct sockaddr))==-1)myerr(3,"FAiled to bind tcp");
			if(listen(tcp_fp,myMaxTCP)==-1)myerr(4,"Fail to set maxConnects");
			
			//Comunication
			get_start(0);
			myfpClose();
		//if
			//tcp SA join Ring
			//tcp himself join Ring (new ring)

		//if
			//InRing
			//Err
	#endif
	exit(0);
}
