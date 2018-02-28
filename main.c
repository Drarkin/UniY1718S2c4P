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

void myerr(int code,char* msg){
	fprintf(stderr,"Err_%d",code);
	if (msg!=NULL) fprintf(stderr,":%s",msg);
	fprintf(stderr,"\n");
	exit(-code);
	}
//configuration of App

//Geral Default Values
char	DV_csName[]="tejo.tecnico.ulisboa.pt";
int	DV_cspt=59000;


struct hostent *csh;
struct in_addr *csa;

//Config Params [CentralServer]
char 		csip[16];	// ipv4 string max size
unsigned int 	cspt=DV_cspt;	//port

#ifdef app_reqserv
	//nothing especific
#elif app_service
	//Default Values
	int	DV_spt=59000;
	//Config Params
	unsigned int	id;
	char		ip[16];
	unsigned int	upt;
	unsigned int	tpt;
#else
	#error app type not defined
	#include <Error_App_NotDef>
#endif

void appConfg (char **arg){
	//arg is a vector of string,
	//arg last pointer must be NULL
	int i=0;
	char *SName=DV_csName;//CentralServer Name
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
			ip=atoi(argv[i]);
		}else   if (strcmp(argv[i],"-u)==0 && argc>i){
                        i++;
			argCheck++;
			upt=atoi(argv[i]);
                }else   if (strcmp(argv[i],"-t")==0 && argc>i){
                        i++;
			argCheck++;
			tpt=atoi(argv[i]);
                }
		#endif
		iif (strcmp(argv[i],"-i")==0 && argc>i){
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
	if (h==NULL) myerr(2,"Fail to gethostbyname");
	csa=(struct in_addr*)h->h_addr_list[0];
				   
	#ifdef debugInfo
		#ifdef app_service
		fprintf(stderr,"id: %d\nip: %d\nupt: %d\ntpt: %d\n",id,ip,upy,tpt);
		#endif
	fprintf(stderr,"CentralServer: %s:c%d\n",SName,cspt);
	fprintf(stderr,"\tofficial host name: %s\n",h->h_name);
	fprintf(stderr,"\tinternet address: %s (%08lX)\n",inet_ntoa(*a),(long unsigned int)ntohl(a->s_addr));
 	#endif
	return;
}
int main (int argc, char **argv) {
	appConfig(argv);
/**LAb1 code	
//jefc
        int SPort=58000;//default value
        char *SName="tejo";//default value
	char msg[1025];
	int i=0;
	int auxi=0;
	int Stimes=1;
	while (argv[i]!=NULL){
		if (strcmp(argv[i],"-m")==0 && argc>i){
			i++;
			auxi=strlen(argv[i]);
			if (auxi>1024)strncpy(msg,argv[i],1024);
			else strcpy(msg,argv[i]); 
		}else	if (strcmp(argv[i],"-n")==0 && argc>i){
			i++;
			SName=argv[i];
		}else   if (strcmp(argv[i],"-p")==0 && argc>i){
                        i++;
                        SPort=atoi(argv[i]);
                }else   if (strcmp(argv[i],"-c")==0 && argc>i){
                        i++;
                        Stimes=atoi(argv[i]);
                }

		i++;
	}
	if ( auxi==0){
		auxi=7;
		strcmp(msg,"Hello!\n");
	}
//First Part
	struct hostent *h = gethostbyname(SName);
	struct in_addr *a;
	if (h==NULL) myerr(1,"Fail to gethostbyname");
	fprintf(stdout,"official host name: %s\n",h->h_name);
	a=(struct in_addr*)h->h_addr_list[0];
	fprintf(stdout,"internet address: %s (%08lX)\n",inet_ntoa(*a),(long unsigned int)ntohl(a->s_addr));
 	
//Second Part
	int fd, n;
	struct sockaddr_in addr;

	fd=socket(AF_INET,SOCK_DGRAM,0);//UDP socket
	if(fd==-1)myerr(2,"Fail in socket()");//error
	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr=*a;//MachineIP
	addr.sin_port=htons(SPort);
	while(0!=Stimes--){
		n=sendto(fd,msg,auxi,0,(struct sockaddr*)&addr,sizeof(addr));
		if(n==-1)myerr(2,"Fail to send");
		fprintf(stdout,">Sent! (%d)\n",Stimes);
	}
//Third Part
	int addrlen;
	char buffer[128];

	addrlen=sizeof(addr);
	n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
	if(n==-1)myerr(3,"Fail to recive data");//error
	write(1,"echo: ",6);//stdout
	write(1,buffer,n);
	write(1,"\n",1);
	close(fd);
//End Lab1Code**/
	exit(0);
}
