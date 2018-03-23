		   
#include "AppServRingVar.h"
#include "GeralCom.h"

#define RingMsgSize_NEW 32
#define RingMsgSize_TOKEN 40
/**DATA*/
const char *GetTypeVectorName[]={"uno","duo","mul","halfway"};
struct RingInfoType RingInfo;
void Ring_SetA(char *A_IP,int A_Id,int A_Port){
	//set addr data correctly
		if (A_Port>65535){
			printf("[Warning] Port value is too big! (Port:%d->%d)\n",A_Port,(short)A_Port);
		}
		A_IP[MYIPSIZE-1]='\0';//delimit string case it was overflowed
	//Save data
	strncpy(RingInfo.A_IP,A_IP,16);
	RingInfo.A_Id=A_Id;
	RingInfo.A_Port=A_Port;
	RingInfo.after.sin_family=AF_INET;
	RingInfo.after.sin_port=htons(A_Port);
	RingInfo.after.sin_addr.s_addr=inet_addr(A_IP);
	}
void Ring_SetB(char *B_IP,int B_Id,int B_Port){
	//set addr data correctly
		if (B_Port>65535){
			printf("[Warning] Port value is too big! (Port:%d->%d)\n",B_Port,(short)B_Port);
		}
		B_IP[MYIPSIZE-1]='\0';//delimit string case it was overflowed
	//Save data
	strncpy(RingInfo.B_IP,B_IP,MYIPSIZE);
	RingInfo.B_Id=B_Id;
	RingInfo.B_Port=B_Port;
	RingInfo.before.sin_family=AF_INET;
	RingInfo.before.sin_port=htons(B_Port);
	RingInfo.before.sin_addr.s_addr=inet_addr(B_IP);
	}
/**/
int Ring(int fd2read,struct sockaddr_in *addr,int myId){
	switch (RingInfo.type){
		case uno:
			return CreateRing(fd2read,addr,myId);
			break;
		case halfway:
			return OuroborosTail(fd2read,addr,myId);
			break;
		case duo:
			
			break;
		case mul:
			
			break;
		default:
			#ifdef debug
				fprintf("[CRITICAL-Ring] OUTSTATE!\n");
			#endif
			return -2;
	}
	close(fd2read);//this to close connections for states that aren't implemented yet
	return -1;
}
int JoinRing(int ServId,int myId,char *myIP,int myPort){
	//Join a ring or other server to form a ring
	struct sockaddr_in *addr=&RingInfo.after;
	char msgBuffer[RingMsgSize_NEW];//worst msg size plus 1
	int tcp_fd=socket(AF_INET,SOCK_STREAM,0);
	int n;
	if (tcp_fd<0){//in case of bad file descriptor Abort
		#ifdef debug
			fprintf("[ERROR-JoinRing] Bad file descriptor!\n");
		#endif
		return tcp_fd;
	}
	//send a join request
	if(0==connect(tcp_fd,(struct sockaddr*)addr,sizeof(*addr))){
		//Sucess
		//Save Addr to RingInfo 
			/**This is called for the new Server, so the addr is from the following server. We save it in A.
			Later a diferent server will try conect to this one to close the ring, it will be B*/
		/*//the Addr of A should already be set previous so we can connect using the addr
		RingInfo.after=(*addr);	
		RingInfo.A_Id=ServId;
		strncpy(RingInfo.A_IP,inet_ntoa((*addr).sin_addr),16);
		RingInfo.A_Port=ntohs((*addr).sin_port);/**/
		//send msg to StartServer
		sprintf(msgBuffer,"NEW %d;%s;%d\n",myId,myIP,(short)myPort);
		n=strlen(msgBuffer);
		#ifdef debug
			fprintf("[INFO-JoinRing] Sent{%s:%d}: %s\n",RingInfo.A_IP,RingInfo.A_Port,msgBuffer);
		#endif
		//if(0<write(tcp_fd,msgBuffer,strlen(msgBuffer))){
		if(n==write(tcp_fd,msgBuffer,n)){
			//Sucess
			RingInfo.type=halfway;
			RingInfo.A_fd=tcp_fd;//saves fd
			return tcp_fd;//retorns the fd of the new conection (type A -  conectes to the server after this one)
		}else{
			//Failed
			#ifdef debug
				fprintf("[ERROR-JoinRing] Failed To Sent, deleting socket!\n");
			#endif
			close(tcp_fd);
			return -1;
		}
	}else{
		//Failed
		#ifdef debug
			fprintf("[ERROR-JoinRing] Failed To connect!\n\tDestAddr: %d@%s:%d\n",ServId,inet_ntoa((*addr).sin_addr),ntohs((*addr).sin_port)));
		#endif
		return -1;
	}
	#ifdef debug
		fprintf("[CRITICAL-JoinRing] OUTSTATE!\n");
	#endif
	return -2;
}

 int OuroborosHead(int myId){
	 //Finish ring creation by conect end to start
	 struct sockaddr_in *addr=&RingInfo.after;
	 char msgBuffer[RingMsgSize_TOKEN];//worst msg size plus 1
	 int tcp_fd=socket(AF_INET,SOCK_STREAM,0);
	 int n;
	 if (tcp_fd<0){//in case of bad file descriptor Abort
		#ifdef debug
			fprintf("[ERROR-Ouroboros] Bad file descriptor!\n");
		#endif
		return tcp_fd;
	}
	if(0==connect(tcp_fd,(struct sockaddr*)addr,sizeof(*addr))){
		//Sucess
		//send msg to NewServer
		sprintf(msgBuffer,"TOKEN %d;N;%d;%s;%d\n",
			myId,
			RingInfo.A_Id,RingInfo.A_IP,RingInfo.A_Port);
		n=strlen(msgBuffer);
		#ifdef debug
			fprintf("[INFO-JoinRing] Sent{%s:%d}: %s\n",RingInfo.A_IP,RingInfo.A_Port,msgBuffer);
		#endif
		//if(0<write(tcp_fd,msgBuffer,strlen(msgBuffer))){
		if(n==write(tcp_fd,msgBuffer,n)){
			//Sucess
			RingInfo.type=duo;
			RingInfo.A_fd=tcp_fd;//saves fd
			return tcp_fd;//retorns the fd of the new conection (type A -  conectes to the server after this one)
		}else{
			//Failed
			#ifdef debug
				fprintf("[ERROR-Ouroboros] Failed To Sent, deleting socket!\n");
			#endif
			close(tcp_fd);
			return -1;
		}
	}else{
		//Failed
		#ifdef debug
			fprintf("[ERROR-Ouroboros] Failed To connect!\n\tDestAddr: %d@%s:%d\n",ServId,inet_ntoa((*addr).sin_addr),ntohs((*addr).sin_port)));
		#endif
		return -1;
	}
	#ifdef debug
		fprintf("[CRITICAL-Ouroboros] OUTSTATE!\n");
	#endif
	return -2;
}
int OuroborosTail(int B_fd,struct sockaddr_in *B_addr,int myId){
	//add B data to RingInfo
	char msgBuffer[RingMsgSize_TOKEN];//worst msg size plus 1
	char ip[MYIPSIZE];
	int ip1,ip2,ip3,ip4,tpt;
	int id,id2;
	int n=read(B_fd,msgBuffer,RingMsgSize_TOKEN);
	if (n>0){
		//Success! Reads Something at least
		#ifdef debug
			fprintf("[INFO] READ: %s\n",msgBuffer);
		#endif
		if (0!=sscanf(msgBuffer,"TOKEN %i;N;%i;%i.%i.%i.%i;%i\n",&id2,&ip1,&ip2,&ip3,&ip4,&tpt) ){
			if(ip1<=255 && ip2<=255 && ip3<=255 && ip4<=255){//verifies the correct max for each ip field
				//Success!
				sprintf(ip,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
				#ifdef debug
					fprintf("[INFO] DATA: %d -> %d@%s:%d\n",id,id2,ip,tpt);
				#endif
				if (myId=id2){
						//Correct info
						//update B data
						RingInfo.B_fd=B_fd;
						RingInfo.before=(*B_addr);
						RingInfo.B_Id=id2;
						strncpy(RingInfo.B_IP,inet_ntoa((*B_addr).sin_addr),16);
						RingInfo.B_Port=ntohs((*B_addr).sin_port);
						//Update Ring State
						RingInfo.type=duo;
						#ifdef debug
							fprintf("[INFO] Ring Created\n",id,id2,ip,tpt);
						#endif
					}else{
						#ifdef debug
							fprintf("[INFO] Ring FAiled! unexpected ID\n",id,id2,ip,tpt);
						#endif
					}
			}else{
				//Fails match
				#ifdef debug
					fprintf("[INFO] FAILED! Invalid IPv4\n");
				#endif
			}	
		}else{
			//Fails match
			#ifdef debug
				fprintf("[INFO] FAILED! Not Recognized\n");
			#endif
			return 0;
		}
	}else{
		//fails to read
		return 0;
	}
			
}
int CreateRing(int tcp_fdB,struct sockaddr_in *addr,int myId){
	//finalizes ring creation if there wasn't a ring before: RingInfo.type==uno (ring made by one server)
	//Args fd of conection 
	// addr address from source 
	int n;
	int id,tpt;
	char ip[MYIPSIZE];
	short ip1,ip2,ip3,ip4;
	char msgBuffer[RingMsgSize_NEW];//worst msg size plus 1 
	n=read(tcp_fdB,msgBuffer,RingMsgSize_NEW);
	if (n>0){
		//Success! Reads Something at least
		#ifdef debug
			fprintf("[INFO] READ: %s\n",msgBuffer);
		#endif
		if (0!=sscanf(msgBuffer,"NEW %i;%i.%i.%i.%i;%i\n",&id,&ip1,&ip2,&ip3,&ip4,&tpt) ){
			if(ip1<=255 && ip2<=255 && ip3<=255 && ip4<=255){//verifies the correct max for each ip field
				//Success!
				sprintf(ip,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
				#ifdef debug
					fprintf("[INFO] DATA: %d@%s:%d\n",id,ip,tpt);
				#endif
				//in this fucntion we are creating the ring, because of this there will be only two nodes. so A and B data in Ring info are equal
				RingInfo.after=(*addr);	
				RingInfo.A_Id=id;
				strncpy(RingInfo.A_IP,inet_ntoa((*addr).sin_addr),16);
				RingInfo.A_Port=ntohs((*addr).sin_port);/**/
				RingInfo.before=(*addr);	
				RingInfo.B_Id=id;
				strncpy(RingInfo.B_IP,inet_ntoa((*addr).sin_addr),16);
				RingInfo.B_Port=ntohs((*addr).sin_port);/**/
				//Now we will finish the ring  by conect to the other node
				n=OuroborosHead(myId);
				if(-1<n){
					//Sucessfull Conected
					return n;//returns The fd of the new conection
				}else{
					return -1;
				}
			}else{
				//Fails match
				#ifdef debug
					fprintf("[INFO] FAILED! Invalid IPv4\n");
				#endif
			}	
		}else{
			//Fails match
			#ifdef debug
				fprintf("[INFO] FAILED! Not Recognized\n");
			#endif
			return 0;
		}
	}else{
		//fails to read
		return 0;
	}
	#ifdef debug
		fprintf("[CRITICAL-JoinRing] OUTSTATE!\n");
	#endif
	return -2;
}