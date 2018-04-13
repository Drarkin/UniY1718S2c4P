#include "GeralCom.h"

#define printMSG fprintf(stdout,"Ans[%i]{%s:%i}: %s\n",fd,inet_ntoa(addrin.sin_addr),addrin.sin_port,myBuffer);
	char myBuffer [buffersize];
	struct sockaddr_in LastInAddr;
//Func
	void bufferclean(){
		int i;
		for (i=0;i<buffersize;i++) myBuffer[i]='\0';
		return;
	}
	int mySend(char *msg,int fd, struct sockaddr_in addr){
		//new version of mysend()
		int n;
		int s=strlen(msg);
		bufferclean();
		if (buffersize>s) s=buffersize;
		strncpy(myBuffer,msg,s);
		return mysend(fd,addr);
		}
	
	int mysend(int fd, struct sockaddr_in addr){
		//sned the buffer contents to the addr using fd
		int n;
		myBuffer[buffersize-1]='\0';//marks end of string
		n=sendto(fd,myBuffer,strlen(myBuffer),0,(struct sockaddr*)&addr,sizeof(addr));
		if(n==-1)fprintf(stderr,"<<GeralCom>> Failed to send!\n");
		else {
			fprintf(stdout,"Sent\t[%i]{%s:%i}: %s\n",fd,inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),myBuffer);
		}
		return n;
	}
	int myrecv(int fd,struct sockaddr_in *addr){
		int n;
		struct sockaddr_in addrin;
		int addrlen=sizeof(addrin);
		bufferclean();
		n=recvfrom(fd,myBuffer,buffersize,0,(struct sockaddr*)&addrin,&addrlen);
		if(n==-1)fprintf(stderr,"<<GeralCom>> Failed to Recive!\n");
		else {
			fprintf(stdout,"Ans\t[%i]{%s:%i}: %s\n",fd,inet_ntoa(addrin.sin_addr),ntohs(addrin.sin_port),myBuffer);
			if(addr!=NULL){
				if (((*addr).sin_addr.s_addr!=addrin.sin_addr.s_addr) &&((*addr).sin_port!=addrin.sin_port)){
					//deletesd msg if it comes from a diferent addr
					fprintf(stderr,"Not expect msg from incoming address!\n");
					bufferclean();
				}
			}
		}
		LastInAddr=addrin;
		return n;
	}