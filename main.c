/*
Made by Drarkin (jefc)
*/

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define app_service

#define debugInfo
#define buffersize 1024
#define IPSIZE 16
#define myMaxTCP 10
#define STDIN 0

#define max(A,B) ((A)>=(B)?(A):(B))
#define myScmp(A) (strncmp(myBuffer,A,strlen(A))==0)

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
	int tcp_fd=-1;
	struct sockaddr_in sudp,stcp;
	//SC adress
	struct sockaddr_in SC_addr;	
	//SC NextRingServer
	struct sockaddr_in ring_addr;//tcp addres to comunivcate with other ring elements
	//Other ServiceServerInfo
	struct app_service_state {//indicates the system state 
		enum {nready,s_ds,s_ds_ok,w_ds,w_ds_ok,s_s,s_s_ok,w_s,w_s_ok,g_s,g_s_ok,ready,busy} state; //have func getEnum
		enum {false,true} ds,ss,ring;// have func getBool
	}AppState;
	int ServX;
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
		if (tcp_fd>=0){ close(tcp_fd);tcp_fd=-1;}
		#endif
		return;
	}

	int mysend(int fp, struct sockaddr_in addr){
		int n;
		n=sendto(fp,myBuffer,strlen(myBuffer),0,(struct sockaddr*)&addr,sizeof(addr));
		if(n==-1)myerr(2,"Fail to send");
			fprintf(stderr,"Sent[%i]: %s\n",fp,myBuffer);
		return n;
	}
	int myrecv(int fp,struct sockaddr_in addr){
		int n;
		int addrlen=sizeof(addr);
		n=recvfrom(fp,myBuffer,buffersize,0,(struct sockaddr*)&addr,&addrlen);
		if(n==-1)myerr(3,"Fail to recive data");//error
		fprintf(stdout,"Ans[%i]: %s\n",fp,myBuffer);
		return n;
	}
#ifdef app_service
	void set_ds(int x){
		sprintf(myBuffer,"SET_DS %i;%i;%s;%i\n",x,id,ip,upt);
		mysend(udp_fp,SC_addr);
		myrecv(udp_fp,SC_addr);
	}
	void withdraw_ds(int x){
		sprintf(myBuffer,"WITHDRAW_DS %i;%i\n",x,id);
		mysend(udp_fp,SC_addr);
		myrecv(udp_fp,SC_addr);//o servidor Central responde! Existe erro
	}
	void set_start (int x){
		sprintf(myBuffer,"SET_START %i;%i;%s;%i\n",x,id,ip,tpt);//verify tpt
		mysend(udp_fp,SC_addr);
		AppState.state=s_s;
	}
	void Ans_set_start(){
		int n;
		myrecv(udp_fp,SC_addr);
		AppState.state=s_s_ok;
		if (0!=sscanf(myBuffer,"OK %i;",&n) && n==id) {AppState.state=s_s_ok;AppState.ss=true;}
	}
	void withdraw_start(int x){
		sprintf(myBuffer,"WITHDRAW_START %i;%i\n",x,id);
		mysend(udp_fp,SC_addr);
		myrecv(udp_fp,SC_addr);
		AppState.ss=false;
	}
	void get_start(int x){
		int addrlen =(int) sizeof(SC_addr);
		sprintf(myBuffer,"GET_START %i;%i\n",x,id);
		mysend(udp_fp,SC_addr);
		AppState.state=g_s;
		return;
	}
	int Ans_get_start(){
		int n;
		myrecv(udp_fp,SC_addr);
		if (0!=sscanf(myBuffer,"OK %i;%i;%s;%i\n",&n,&Oid,Oip,&Otpt) && n==id){
			fprintf(stderr,"AnsData:\n\tid: %i\n\tip %s\n\ttpt %i\n",Oid,Oip,Otpt);
			return 1;
		}
		fprintf(stderr,"Err WrongMsg\n");
		return 0;
	}
	void serv_start(){
		fprintf(stderr,">>Start Exists [%d@%s:%d]\n",Oid,Oip,Otpt);
		
	}
	//AUX FUNV
	
	
	/*********************/
	int userIns(){
		//reads user input from stdin and calls the correct function to execut user command
		int intaux;
		myBuffer[0]='\0';//Reset burffer data
		/*fprintf(stderr,"in function userIns\n");//Debug*/
		intaux=scanf("%[^\n]",myBuffer);
		/*prevent loopbug*/
		/*fprintf(stderr,">>%s<<%d\n",myBuffer,intaux);//debug*/
		if (intaux<=0) {getchar();return 0;}
		
		if (myScmp("quit")||myScmp("exit")){
			if myScmp("exit") 
				if (!(AppState.ds || AppState.ss ))return 1;
				else fprintf(stderr,"still conectect!\n");
			if (AppState.ds==true){
				withdraw_ds(ServX);
			}
			if (AppState.ss==true){
				withdraw_start(ServX);
			}
		}else if(myScmp("join")){
			//entrar no anel do serviço x
			// por omissao entrar no anel disponicel
			ServX=atoi(&myBuffer[4]);
			fprintf(stderr,">>join with id %d\n",ServX);
			get_start(ServX);
		}else if(myScmp("show_state")){
			printf("\tServerState: %i (ss: %i  /  ds: %i)\n",AppState.state,AppState.ss,AppState.ds);
			//print state
		}else if (myScmp("leave")){
			//saida do servidor do anel
		}else if myScmp("set_ds"){
			fprintf(stderr,">>SET_DS!\n");
		}else{
			fprintf(stdout,"Unknow Command!\n");
			fflush(stdout);
		}
		
		
		return 0;
	}
	void appRun(){
		//main code for service functionally
		//wait for new information to be read from any file descriptor and executs the correct answer for that input
		enum {idle,busy} state;//state that controls select
		int fd,newfd,afd;
		fd_set rfds;
		int maxfd,counter;
		//Find max value
		maxfd=(tcp_fd>STDIN)?tcp_fd:STDIN;
		maxfd=(udp_fp>maxfd)?udp_fp:maxfd;
		while(1){
			FD_ZERO(&rfds);
			FD_SET(tcp_fd,&rfds);
			FD_SET(udp_fp,&rfds);
			FD_SET(STDIN,&rfds);
			//FD_SET((int)STDIN,&rfds);//jefc
			if(state==busy){FD_SET(afd,&rfds);maxfd=max(maxfd,afd);}
			
			counter=select(maxfd+1,&rfds,
							(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);				
			if (counter<=0)myerr(158,"Failed in appRun()");
			
			//put code to read STDIN when input is writen
			if (FD_ISSET(STDIN,&rfds)){
				if(userIns())return;
			}
			if (FD_ISSET(tcp_fd,&rfds)){
			}
			if (FD_ISSET(udp_fp,&rfds)){
				switch (AppState.state){
					case g_s:
							if (Ans_get_start())AppState.state=g_s_ok;
							else AppState.state=nready;
 						break;
					case s_s: Ans_set_start();
						break;
					default: break;	
				}
			}
			switch(AppState.state){
				case g_s_ok:
					if (Oid==0){
						set_start(ServX);//para o servico X
					}else{
						serv_start();
						//set next ring address
						//conect to ring
					}
					break;
				case s_s_ok: //caso de 
					//next ring element is himself
					Oid=id;
					strcpy(Oip,ip);
					Otpt=tpt;
					serv_start();//executar o codigo do state g_s_ok para Oid!=0 (entrar no anel)
					AppState.state=nready;
					break;
				default: break;	
			}
			
		}
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
			
			//Comunication
			/*get_start(0);
			set_ds(72673);
			set_start(72673);
			getchar();
			withdraw_ds(72673);
			withdraw_start(72673);/**/
			appRun();
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
