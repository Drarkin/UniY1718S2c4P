#include "GeralCom.h"
	char myBuffer [buffersize];
//Func
	int mysend(int fp, struct sockaddr_in addr){
		int n;
		n=sendto(fp,myBuffer,strlen(myBuffer),0,(struct sockaddr*)&addr,sizeof(addr));
		if(n==-1)fprintf(stderr,"<<GeralCom>> Failed to send!\n");
		else fprintf(stdout,"Sent[%i]: %s\n",fp,myBuffer);
		return n;
	}
	int myrecv(int fp,struct sockaddr_in addr){
		int n;
		int addrlen=sizeof(addr);
		n=recvfrom(fp,myBuffer,buffersize,0,(struct sockaddr*)&addr,&addrlen);
		if(n==-1)fprintf(stderr,"<<GeralCom>> Failed to Recive!\n");
		else fprintf(stdout,"Ans[%i]: %s\n",fp,myBuffer);
		return n;
	}