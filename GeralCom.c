#include "GeralCom.h"
	char myBuffer [buffersize];
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
		if (buffersize>s) s=buffersize;
		strncpy(myBuffer,msg,s);
		return mysend(fd,addr);
		}
	
	int mysend(int fd, struct sockaddr_in addr){
		//sned the buffer contents to the addr using fd
		int n;
		//bufferclean();
		myBuffer[buffersize-1]='\0';//marks end of string
		n=sendto(fd,myBuffer,strlen(myBuffer),0,(struct sockaddr*)&addr,sizeof(addr));
		if(n==-1)fprintf(stderr,"<<GeralCom>> Failed to send!\n");
		else fprintf(stdout,"Sent[%i]: %s\n",fd,myBuffer);
		return n;
	}
	int myrecv(int fd,struct sockaddr_in addr){
		int n;
		int addrlen=sizeof(addr);
		bufferclean();
		n=recvfrom(fd,myBuffer,buffersize,0,(struct sockaddr*)&addr,&addrlen);
		if(n==-1)fprintf(stderr,"<<GeralCom>> Failed to Recive!\n");
		else fprintf(stdout,"Ans[%i]: %s\n",fd,myBuffer);
		return n;
	}