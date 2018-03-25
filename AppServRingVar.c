		   
#include "AppServRingVar.h"
#include "GeralCom.h"

#define RingMsgSize_NEW 100 //20
#define RingMsgSize_TOKEN 100//40

/**DATA*/
const char *GetTypeVectorName[]={"uno","duo","mul","halfway"};
struct RingInfoType RingInfo;
char RingMsgBuffer[RingMsgSize_MAX];
char TOKEN;

enum {idle,busy} Snode,Sring;
void CleanRing (){
	memset((void*)&RingInfo,(int)'\0',sizeof(RingInfo));
	RingInfo.A_fd=-1;
	RingInfo.B_fd=-1;
	return;
}
/**setting state of node/ring*/
 int RingBusy(){return Sring;}
 int RingNodeBusy(){return Snode;}
void RingSetNodeBusy(){ Snode=busy;}
void RingSetNodeIdle(){ Snode=idle;}
void RingSetBusy(){Sring=busy;}
void RingSetIdle(){Sring=idle;}
/**Set Addr in RingInfo*/
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
	RingInfo.A_addr.sin_family=AF_INET;
	RingInfo.A_addr.sin_port=htons(A_Port);
	RingInfo.A_addr.sin_addr.s_addr=inet_addr(A_IP);
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
	RingInfo.B_addr.sin_family=AF_INET;
	RingInfo.B_addr.sin_port=htons(B_Port);
	RingInfo.B_addr.sin_addr.s_addr=inet_addr(B_IP);
	}
/**/
/**Send MSG to next ring member*/
int RingMsgPidgeon(char *msg){
//Continue to pass the msg
	int size;
	int msgSize;
	#ifdef debug
	fprintf(stderr,"[INFO-Pidgeon] Sent %d Bytes{%d@%s:%d}: ",strlen(msg),RingInfo.A_Id,RingInfo.A_IP,RingInfo.A_Port);
	#endif
	size=strlen(msg)+1;
	msgSize=write(RingInfo.A_fd,msg,size);
	if(msgSize==size){
		//Success
		#ifdef debug
		fprintf(stderr,"%s\n",msg);
		#endif
		return RingInfo.A_fd;
	}else{
		//Failed
		#ifdef debug
			fprintf(stderr,"\n[ERROR-Pidgeon] Failed To Send! (%d=%d)\n",msgSize,size);
		#endif
		return ErrRingA;//
	}
}
/**MAnage rings members */
int Ring(int fd2read,struct sockaddr_in *addr,int myId,int StartServer){
	/*
		-1 ignore
		-2 close B
		-3 close A
	*/
	if(fd2read==RingInfo.B_fd){
		//read msg fromring member
		return RingToken(myId);
	}
		//read message from unknown
	switch (RingInfo.type){
		case uno:
			if (StartServer)return CreateRing(fd2read,addr,myId);;
			break;
		case halfway:
			return OuroborosTail(fd2read,addr,myId);
			break;
		case duo:
		case mul:
			if (StartServer)return NewServer(fd2read,myId);
			break;
		default:
			#ifdef debug
				fprintf(stderr,"[CRITICAL-Ring] OUTSTATE!\n");
			#endif
			return ErrRingNewfd; //close connectiond that don't belong to a state
	}	
	#ifdef debug
		fprintf(stderr,"[WARNING-Ring] nothing to do!\n");
	#endif
	return ErrRingIngnore;
	
}
int JoinRing(int ServId,int myId,char *myIP,int myPort){
	//Join a ring (or other server) to form a (new) ring
	/*
	Before using this function, it's necessary to use:
		Ring_SetA(Taerget_STR_ip,int_id,int_Otpt);
	*/
	struct sockaddr_in *addr=&RingInfo.A_addr;
	char msgBuffer[RingMsgSize_NEW];//worst msg size plus 1
	int tcp_fd=socket(AF_INET,SOCK_STREAM,0);
	int n;
	if (tcp_fd<0){//in case of bad file descriptor Abort
		#ifdef debug
			fprintf(stderr,"[ERROR-JoinRing] Bad file descriptor!\n");
		#endif
		return tcp_fd;
	}
	//send a join request
	if(0==connect(tcp_fd,(struct sockaddr*)addr,sizeof(*addr))){
		//Sucess
		//Save Addr to RingInfo 
			/**This is called for the new Server, so the addr is from the following server. We save it in A.
			Later a diferent server will try conect to this one to close the ring, it will be B*/
		/*//the Addr of A should already be set previous so we can connect using the addr*/
		//send msg to StartServer
		sprintf(msgBuffer,"NEW %d;%s;%d\n\0",myId,myIP,(short)myPort);
		n=strlen(msgBuffer)+1;
		#ifdef debug
			fprintf(stderr,"[INFO-JoinRing] Sent{%d:%s:%d}: %s\n",RingInfo.A_Id,RingInfo.A_IP,RingInfo.A_Port,msgBuffer);
		#endif
		//if(0<write(tcp_fd,msgBuffer,strlen(msgBuffer))){
		if(n==write(tcp_fd,msgBuffer,n)){
			//Sucess
			RingInfo.type=halfway;
			RingInfo.A_fd=tcp_fd;//saves fd
			return tcp_fd;//retorns the fd of the new conection (type A -  conectes to the server A_addr this one)
		}else{
			//Failed
			#ifdef debug
				fprintf(stderr,"[ERROR-JoinRing] Failed To Sent, deleting socket!\n");
			#endif
			close(tcp_fd);
			return -1;
		}
	}else{
		//Failed
		#ifdef debug
			fprintf(stderr,"[ERROR-JoinRing] Failed To connect!\n\tDestAddr: %d@%s:%d\n",ServId,inet_ntoa((*addr).sin_addr),ntohs((*addr).sin_port));
		#endif
		return -1;
	}
	#ifdef debug
		fprintf(stderr,"[CRITICAL-JoinRing] OUTSTATE!\n");
	#endif
	return -2;
}
int CloseRingAfterMemberLeaves(int myId){
	/*iIn this function we will close the ring again, after the sucessor of this node leave the ring
	first we will test if the node leaving is the sucessor of this node, 
		if it is the successor we will do something like joinRing,*/
		int n;
	int B_fd=RingInfo.B_fd;//read from previous ringNode
	int msgSize;
	struct RingInfoType RingInfoBackup;
	int id,type,id2,ip1,ip2,ip3,ip4,tpt;
	char ip[MYIPSIZE]="---.---.---.---";
	char *msgBuffer=RingMsgBuffer;//change msgBuffer to RingMSgBuffer
	n=sscanf(RingMsgBuffer,"TOKEN %d;%c;%d;%d.%d.%d.%d;%d",&id,&type,&id2,&ip1,&ip2,&ip3,&ip4,&tpt);
	if (n!=8) return ErrRingIngnore;
	if(ip1<=255 && ip2<=255 && ip3<=255 && ip4<=255){
		if (id==RingInfo.A_Id &&id==RingInfo.B_Id){
				//ring dismantled
				close(RingInfo.B_fd);
				close(RingInfo.A_fd);
				CleanRing ();
				return ErrRingIngnore;
			}
		if(id==RingInfo.A_Id){//my sucessor is leaving
			sprintf(ip,"%d.%d.%d.%d\0",ip1,ip2,ip3,ip4);
			close(RingInfo.A_fd);RingInfo.A_fd=-1;
			Ring_SetA(ip,id2,tpt);
			if(0>OuroborosHead(myId)){
				//FAilure to connect with Sucessor
				return ErrRingA;
			}else{
				//Sucess to connect with new Server
				#ifdef debug
					fprintf(stderr,"[INFO-RingToken] Success to conect to Successor server\n",RingInfo.A_IP,RingInfo.A_Port,msgBuffer);
				#endif
				if (ErrRingA==RingMsgPidgeon(msgBuffer))
					return ErrRingNoRing;//should be critical no node before
				RingInfo.type=uno;
				return RingInfo.A_fd;
			}
		}else if(id==RingInfo.B_Id){// previous node is leaving
			close(RingInfo.B_fd);RingInfo.B_fd=-1;
			RingInfo.type=halfway;
			return RingInfo.A_fd;
			
		}else{
			//Continue to pass the msg
			return RingMsgPidgeon(msgBuffer);
		}	
	}else{
		//Fails match
		#ifdef debug
			fprintf(stderr,"[ERROR-InsertNewRingMember] FAILED! Invalid IPv4: %d.%d.%d.%d\n",ip1,ip2,ip3,ip4);
		#endif
		return -1;
	}	
		
	
}
 int OuroborosHead(int myId){
	 //return positive in case of success
	 //Finish ring creation by conect end to start
	 struct sockaddr_in *addr=&RingInfo.A_addr;
	 char msgBuffer[RingMsgSize_TOKEN];//worst msg size plus 1
	 int tcp_fd=socket(AF_INET,SOCK_STREAM,0);
	 int n;
	 if (tcp_fd<0){//in case of bad file descriptor Abort
		#ifdef debug
			fprintf(stderr,"[ERROR-OuroborosHead] Bad file descriptor!\n");
		#endif
		return tcp_fd;//negative
	}
	
	#ifdef debug
		fprintf(stderr,"[INFO-OuroborosHead] Sent{%d@%s:%d}: ",RingInfo.A_Id,RingInfo.A_IP,RingInfo.A_Port);
	#endif
	if(0==connect(tcp_fd,(struct sockaddr*)addr,sizeof(*addr))){
		//Sucess
		//send msg to NewServer
		sprintf(msgBuffer,"TOKEN %d;N;%d;%s;%d\n\0",
			myId,
			RingInfo.A_Id,RingInfo.A_IP,RingInfo.A_Port);
		n=strlen(msgBuffer)+1;
		//if(0<write(tcp_fd,msgBuffer,strlen(msgBuffer))){
		if(n==write(tcp_fd,msgBuffer,n)){
			//Sucess
			#ifdef debug
				fprintf(stderr,"%s\n",msgBuffer);
			#endif
			RingInfo.type=duo;
			RingInfo.A_fd=tcp_fd;//saves fd
			return tcp_fd;//retorns the fd of the new conection (type A -  conectes to the server A_addr this one)
		}else{
			//Failed
			#ifdef debug
				fprintf(stderr,"\n[ERROR-OuroborosHead] Failed To Sent, deleting socket!\n");
			#endif
			close(tcp_fd);
			return -1;
		}
	}else{
		//Failed
		#ifdef debug
			fprintf(stderr,"[ERROR-OuroborosHead] Failed To connect!\n\tDestAddr: %d@%s:%d\n",RingInfo.A_Id,inet_ntoa((*addr).sin_addr),ntohs((*addr).sin_port));
		#endif
		return -1;
	}
	#ifdef debug
		fprintf(stderr,"[CRITICAL-OuroborosHead] OUTSTATE!\n");
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
			fprintf(stderr,"[INFO-OuroborosTail] READ: %s\n",msgBuffer);
		#endif
		if (7==sscanf(msgBuffer,"TOKEN %d;N;%d;%d.%d.%d.%d;%d",&id,&id2,&ip1,&ip2,&ip3,&ip4,&tpt) ){
			if(ip1<=255 && ip2<=255 && ip3<=255 && ip4<=255){//verifies the correct max for each ip field
				//Success!
				sprintf(ip,"%d.%d.%d.%d\0",ip1,ip2,ip3,ip4);
				#ifdef debug
					fprintf(stderr,"[INFO-OuroborosTail] DATA: %d -> %d@%s:%d\n",id,id2,ip,tpt);
				#endif
				if (myId=id2){
						//Correct info
						//update B data
						//Wrong info because source addr change when packages moves in network
						RingInfo.B_fd=B_fd;
						RingInfo.B_addr=(*B_addr);
						RingInfo.B_Id=id;
						strncpy(RingInfo.B_IP,inet_ntoa((*B_addr).sin_addr),16);
						RingInfo.B_Port=ntohs((*B_addr).sin_port);
						//Update Ring State
						RingInfo.type=duo;
						#ifdef debug
							fprintf(stderr,"[INFO-OuroborosTail] Ring Created\n",id,id2,ip,tpt);
						#endif
						return B_fd;
					}else{
						#ifdef debug
							fprintf(stderr,"[INFO-OuroborosTail] Ring FAiled! unexpected ID\n",id,id2,ip,tpt);
						#endif
						return -1;
					}
			}else{
				//Fails match
				#ifdef debug
					fprintf(stderr,"[INFO-OuroborosTail] FAILED! Invalid IPv4\n");
				#endif
				return -1;
			}	
		}else{
			//Fails match
			#ifdef debug
				fprintf(stderr,"[INFO-OuroborosTail] FAILED! Not Recognized\n");
			#endif
			return -1;
		}
	}else{
		//failure to read
		#ifdef debug
			fprintf(stderr,"[INFO-OuroborosTail] FAILED! fail read\n");
		#endif
		return -1;
	}
	
	#ifdef debug
		fprintf(stderr,"[CRITICAL-OuroborosTail] OUTSTATE!\n");
	#endif
	return -1;	
}
int CreateRing(int tcp_fdB,struct sockaddr_in *addr,int myId){
	/**Creates a ring case there wansn't a Ring before*/
	//Args fd of conection 
	// addr address from source 
	int n;
	int id,tpt;
	char ip[MYIPSIZE];
	int ip1,ip2,ip3,ip4;
	char msgBuffer[RingMsgSize_NEW];//worst msg size plus 1 
	n=read(tcp_fdB,msgBuffer,RingMsgSize_NEW);
	if (n>0){
				//Success! Reads Something at least
		#ifdef debug
			fprintf(stderr,"[INFO-CreateRing] READ: %s\n",msgBuffer);
		#endif
		if (6==sscanf(msgBuffer,"NEW %i;%d.%d.%d.%d;%i\n",&id,&ip1,&ip2,&ip3,&ip4,&tpt) ){
			if(ip1<=255 && ip2<=255 && ip3<=255 && ip4<=255){//verifies the correct max for each ip field
				//Success!
				sprintf(ip,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
				#ifdef debug
					fprintf(stderr,"[INFO-CreateRing] DATA: %d@%s:%d\n",id,ip,tpt);
				#endif
				//in this fucntion we are creating the ring, because of this there will be only two nodes. so A and B data in Ring info are equal
				Ring_SetA(ip,id,tpt);
				Ring_SetB(ip,id,tpt);
				//Now we will finish the ring  by conect to the other node
				n=OuroborosHead(myId);
				if(-1<n){
					//Sucessfull Conected
					RingInfo.B_fd=tcp_fdB;
					return n;//returns The fd of the new conection
				}else{
					#ifdef debug
						fprintf(stderr,"[INFO-CreateRing] FAILED! head didn't bite tail\n");
					#endif
					return -1;
				}
			}else{
				//Fails match
				#ifdef debug
					fprintf(stderr,"[INFO-CreateRing] FAILED! Invalid IPv4\n");
				#endif
				return -1;
			}	
		}else{
			//Fails match
			#ifdef debug
				fprintf(stderr,"[INFO-CreateRing] FAILED! Not Recognized\n");
			#endif
			return -1;
		}
	}else{
		//fails to read
		return -1;
	}
	#ifdef debug
		fprintf(stderr,"[CRITICAL--CreateRing] OUTSTATE!\n");
	#endif
	return -2;
}
int NewServer(int tcp_fdB,int myID){
	//New server try to conect to ring
	//Args fd of conection 
	// addr address from source 
	int n;
	int id,tpt;
	char ip[MYIPSIZE];
	int ip1,ip2,ip3,ip4;
	char msgBuffer[RingMsgSize_TOKEN];//worst msg size plus 1 
	int msgSize;
	int BId,Nid,Nip1,Nip2,Nip3,Nip4,Ntpt;
	n=read(tcp_fdB,msgBuffer,RingMsgSize_TOKEN);
	if (n>0){
		//Success in read msg
		#ifdef debug
			fprintf(stderr,"[INFO-NewServer] READ: %s\n",msgBuffer);
		#endif
		if (6==sscanf(msgBuffer,"NEW %i;%d.%d.%d.%d;%i\n",&id,&ip1,&ip2,&ip3,&ip4,&tpt) ){
			if(ip1<=255 && ip2<=255 && ip3<=255 && ip4<=255){//verifies the correct max for each ip field
			//MSG matched protocol
				//Success!
				sprintf(ip,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
				#ifdef debug
					fprintf(stderr,"[INFO-NewServer] DATA: %d@%s:%d\n",id,ip,tpt);
				#endif
				//Warn ring of new member
				//Start pass token
				sprintf(msgBuffer,"TOKEN %d;N;%d;%s;%d\n",myID,id,ip,tpt);
				msgSize=strlen(msgBuffer)+1;
				if(msgSize==write(RingInfo.A_fd,msgBuffer,msgSize)){
					#ifdef debug
						fprintf(stderr,"[INFO-NewServer] SENT{%d@%s:%d}: %s\n",id,ip,tpt,msgBuffer);
					#endif
					//obter confirmaçao do atecedente
					msgSize=read(RingInfo.B_fd,msgBuffer,RingMsgSize_TOKEN);
					//Sucessfull Conected
					if(msgSize>0){
						#ifdef debug
							fprintf(stderr,"[INFO-NewServer] Recv: %s\n",msgBuffer);
						#endif
						sscanf(msgBuffer,"TOKEN %d;K;%d;%d.%d.%d.%d;%d\n\0",&BId,&Nid,&Nip1,&Nip2,&Nip3,&Nip4,&Ntpt);
						if(Nid==id && Nip1==ip1 && Nip2==ip2 && Nip3==ip3 && Nip4==ip4 && Ntpt==tpt && RingInfo.B_Id==BId){
							//check that ring formed with the new server
							close(RingInfo.B_fd);
							Ring_SetB(ip,id,tpt);
							RingInfo.B_fd=tcp_fdB;
						}else{
						#ifdef debug
							fprintf(stderr,"[INFO-NewServer] Wrong Data: %d;K;%d;%d.%d.%d.%d;%d\n",BId,Nid,Nip1,Nip2,Nip3,Nip4,Ntpt);
							fprintf(stderr,"[INFO-NewServer] expected  : %d;K;%d;%d.%d.%d.%d;%d\n",RingInfo.B_Id,id,ip1,ip2,ip3,ip4,tpt);
						#endif
						return -1;
						}
					}else{
						//fail to recive
						#ifdef debug
							fprintf(stderr,"[INFO-NewServer] FAILED to Recv\n");
						#endif
						return -1;
					}
					//SUCCESS
					return tcp_fdB;//returns The fd of the new conection
				}else{
					#ifdef debug
						fprintf(stderr,"[INFO-NewServer] FAILED to send: %s\n",msgBuffer);
					#endif
					return -1;
				}
			}else{
				//Fails match
				#ifdef debug
					fprintf(stderr,"[INFO-NewServer] FAILED! Invalid IPv4\n");
				#endif
				return -1;
			}	
		}else{
			//Fails match
			#ifdef debug
				fprintf(stderr,"[INFO-NewServer] FAILED! Not Recognized\n");
			#endif
			return -1;
		}
	}else{
		//fails to read
		return -1;
	}
	#ifdef debug
		fprintf(stderr,"[CRITICAL--NewServer] OUTSTATE!\n");
	#endif
	return -2;
}
int InsertNewRingMember( int myId){
	//insert new server 
	//breaks old ring after sucessfull connect to the new server to create a new ring
	int n;
	int B_fd=RingInfo.B_fd;//read from previous ringNode
	int msgSize;
	struct RingInfoType RingInfoBackup;
	int id,type,id2,ip1,ip2,ip3,ip4,tpt;
	char ip[MYIPSIZE]="---.---.---.---";
	char *msgBuffer=RingMsgBuffer;//change msgBuffer to RingMSgBuffer
	n=sscanf(RingMsgBuffer,"TOKEN %d;%c;%d;%d.%d.%d.%d;%d",&id,&type,&id2,&ip1,&ip2,&ip3,&ip4,&tpt);
	if (n!=8) return ErrRingIngnore;
	if(ip1<=255 && ip2<=255 && ip3<=255 && ip4<=255){
		if(id==RingInfo.A_Id){
			sprintf(ip,"%d.%d.%d.%d\0",ip1,ip2,ip3,ip4);
			RingInfoBackup=RingInfo;
			Ring_SetA(ip,id2,tpt);
			if(0>OuroborosHead(myId)){
				//FAilure to connect with new Server
				RingInfo=RingInfoBackup;//faz o back up da informação correcta do anel
				#ifdef debug
					fprintf(stderr,"[INFO-InsertNewRingMember] Fail to conect to new server\n",RingInfo.A_IP,RingInfo.A_Port,msgBuffer);
				#endif
				/*Avisar SERVIDOR START QUE FALHOU*/
				/** Type F warns that the head of ring (server before StartServer) failed to connect to the new server, aborts new server */
				/***TOKEN myID;F;id2;pip2;tpt2\n**/
				sprintf(msgBuffer,"TOKEN %d;F;%d;%d.%d.%d.%d;%d\n\0",myId,id2,ip1,ip2,ip3,ip4,tpt);
				msgSize=strlen(msgBuffer)+1;
				if(msgSize==write(RingInfo.A_fd,msgBuffer,msgSize)){
					//Success
					#ifdef debug
						fprintf(stderr,"[INFO-InsertNewRingMember] Sent{%s:%d}: %s\n",RingInfo.A_IP,RingInfo.A_Port,msgBuffer);
					#endif
				}else{
					//Failed
					#ifdef debug
						fprintf(stderr,"[Critical-InsertNewRingMember] Failed To A_node! exit(0)!\n");
					#endif
					#ifdef ExitOnCritic
						exit(0);
					#endif
					return -1;
				}
				return 0;
			}else{
				//Sucess to connect with new Server
				#ifdef debug
					fprintf(stderr,"[INFO-InsertNewRingMember] Success to conect to new server\n",RingInfo.A_IP,RingInfo.A_Port,msgBuffer);
				#endif
				/*Avisar SERVIDOR START QUE Success*/
				/** Type F warns that the head of ring (server before StartServer) failed to connect to the new server, aborts new server */
				/***TOKEN myID;F;id2;pip2;tpt2\n**/
				sprintf(msgBuffer,"TOKEN %d;K;%d;%d.%d.%d.%d;%d\n\0",myId,id2,ip1,ip2,ip3,ip4,tpt);
				msgSize=strlen(msgBuffer)+1;
				if(msgSize==write(RingInfoBackup.A_fd,msgBuffer,msgSize)){
					//Success
					#ifdef debug
						fprintf(stderr,"[INFO-InsertNewRingMember] Sent{%s:%d}: %s\n",RingInfo.A_IP,RingInfo.A_Port,msgBuffer);
					#endif
				}else{
					//Failed
					#ifdef debug
						fprintf(stderr,"[Critical-InsertNewRingMember] Failed To A_node! exit(0)!\n");
					#endif
					return -1;
				}
				close(RingInfoBackup.A_fd);//close old A_fd connection
				return RingInfo.A_fd;
			}
		}else{
			//Continue to pass the msg
			return RingMsgPidgeon(msgBuffer);
		}	
	}else{
		//Fails match
		#ifdef debug
			fprintf(stderr,"[INFO-InsertNewRingMember] FAILED! Invalid IPv4: %d.%d.%d.%d\n",ip1,ip2,ip3,ip4);
		#endif
		return ErrRingIngnore;
	}	
}

char* RingReadMSG(){
	//read msg from previous ring member (tcp of B)
	//return 1 on success
	//str
	//worst msg size plus 1
	int msgSize;
	fprintf(stderr,"[INFO-RingReadMSG] READ(%d): ",RingInfo.B_fd);
	for (msgSize=0;msgSize<RingMsgSize_MAX;msgSize++){ RingMsgBuffer[msgSize]='\0';}
	msgSize=read(RingInfo.B_fd,RingMsgBuffer,RingMsgSize_MAX-1);
	if (msgSize>0){
		fprintf(stderr,"%s\n",RingMsgBuffer);
		return RingMsgBuffer;
	}else{
		#ifdef debug
			fprintf(stderr,"\n[INFO-RingReadMSG] Failed To read! Close B_fd: %d\n",RingInfo.B_fd);
		#endif
		close(RingInfo.B_fd);
		RingInfo.B_fd=-1;
		return NULL;
	}
}
int RingToken(int myId){
	//add B data to RingInfo
	char msgBuffer[RingMsgSize_TOKEN];//worst msg size plus 1
	char ip[MYIPSIZE]="---.---.---.---";
	char type;
	int ip1,ip2,ip3,ip4,tpt;
	int id,id2;
	int n;
	int B_fd=RingInfo.B_fd;//read from previous ringNode
	int msgSize;
	struct RingInfoType RingInfoBackup;
	
	/*n=sscanf(msgBuffer,"TOKEN %d;%c;%d;%d.%d.%d.%d;%d",&id,&type,&id2,&ip1,&ip2,&ip3,&ip4,&tpt);/**/
	n=sscanf(RingMsgBuffer,"TOKEN %d;%c",&id,&type);
	if (2==n){
		TOKEN=type;//allows external code to see what was the type of token
		if(id==myId) fprintf(stderr,"[INFO-RingTokenTOKEN] has returned!\n");
		switch(type){
			case 'N': if(id!=myId){return InsertNewRingMember(myId);}break;//ingore if msg have server id
			case 'O':if(id!=myId){return CloseRingAfterMemberLeaves(myId);}break;
			case 'S':if(id==myId){sprintf(msgBuffer,"TOKEN %d;I\n",myId);return RingMsgPidgeon(msgBuffer);}if(RingNodeBusy()){break;}return ErrRingIngnore;
			case 'I':RingSetBusy();break;
			case 'D':if(myId!=id){TOKEN='d';RingSetIdle();}if (RingNodeBusy()){RingSetIdle();break;}if(myId>id){break;}return ErrRingIngnore;//executa como dito no enunciado
			case 'T':break;
			case 'M':break;
			default: return ErrRingIngnore;//ignore mensagens nao programadas
		}
		if(id==myId)return ErrRingIngnore;//nao renvia mensagens proprias
		return RingMsgPidgeon(RingMsgBuffer);//renvia mensagens
	}
	#ifdef debug
			fprintf(stderr,"[INFO-RingToken] FAILED! Not Recognized\n");
	#endif
	return  ErrRingIngnore;//ignore mensagens nao programadas;	
}