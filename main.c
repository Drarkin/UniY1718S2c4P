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

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

void myerr(int code,char* msg){
	fprintf(stderr,"Err_%d",code);
	if (msg!=NULL) fprintf(stderr,":%s",msg);
	fprintf(stderr,"\n");
	exit(-code);
	}
int main (int argc, char **argv) {
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

	exit(0);
}
