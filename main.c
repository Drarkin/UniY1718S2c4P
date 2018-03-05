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


void myerr(int code,char* msg){
	fprintf(stderr,"Err_%d",code);
	if (msg!=NULL) fprintf(stderr,":%s",msg);
	fprintf(stderr,"\n");
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
char 		csip[16];	// ipv4 string max size
unsigned int 	cspt;	//port

#ifdef app_service
	//Default Values
	int	DV_spt=59000;
	//Config Params
	unsigned int	id;
	char		*ip;
	unsigned int	upt;
	unsigned int	tpt;
#endif
#ifndef app_reqserv
	#ifndef app_service
		#error app type not defined
		#include <Error_App_NotDef>
	#endif
#endif

//functions

#ifdef app_service
	void set_ds(int x){
		stringf(mybuffer,"SET_DS %i;%i;%s;%i\n",x,id,ip,upt);
	}
	void withdraw(int x){
		string(mybuffer,"WITHDRAW %i;%i\n",x,id);
	}
	void set_start (int x){
		string(mybuffer,"SET_START %i;%i;%s;%i\n",x,id,ip,tcp);
	}
	void withdraw_start(int x){
		string(mybuffer,"WITHDRAW_START %i;%i\n",x,id);
	}
	void get_Start(int x){
		string(mybuffer,"GET_START %i;%i\n",x,id);
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
			int SC_fp;
			struct sockaddr_in SC_addr;

			SC_fp=socket(AF_INET,SOCK_DGRAM,0);//UDP socket
			if(SC_fp==-1)myerr(2,"SocketFail Err001");//error
			memset((void*)&SC_addr,(int)'\0',sizeof(SC_addr));
				SC_addr.sin_family=AF_INET;
				SC_addr.sin_addr=*csa;//MachineIP
				SC_addr.sin_port=htons(cspt);
			//Comunication
		
			/*while(0!=Stimes--){
				n=sendto(fd,msg,auxi,0,(struct sockaddr*)&addr,sizeof(addr));
				if(n==-1)myerr(2,"Fail to send");
				fprintf(stdout,">Sent! (%d)\n",Stimes);*/
			close(SC_fp);
		//if
			//tcp SA join Ring
			//tcp himself join Ring (new ring)

		//if
			//InRing
			//Err
	#endif
	exit(0);
}
