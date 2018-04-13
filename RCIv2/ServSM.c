#include "ServSM.h"
#define SM_TimeOut -1

#define SM_InStd 0
#define SM_InUdp 1
#define SM_InTcp 2
#define SM_InRing 3
#define SM_InVoid -1

#define SM_SysOff 0
#define SM_SysOn 1

#define NotImpl; fprintf(stdout,"Not Implemented!\n");
void ServRun();

void fdSelector();
int StateMachine(int fd,int fdType);
int SM_LocalUserInput(int fd);
/** <-CODE-> **/
void ServRun(){ return fdSelector();}
	
void fdSelector(){
	//Selects the fd to be read by system
	//
		int StateCode;
		int errnum;
		int fd,newfd,afd;
		fd_set rfds;
		struct timeval tv = {60, 0};   // select timemout

		int maxfd,counter;
		int tpc_in_AddrSize;
		RingMsgBuffer=myBuffer;//changiung ring buffer to be the same buffer as the rest
		//Find max value
		maxfd=max(tcp_fd,STDIN);
		maxfd=max(udp_fp,maxfd);
		CleanRing ();
		while(1){
			FD_ZERO(&rfds);
			FD_SET(tcp_fd,&rfds);
			FD_SET(udp_fp,&rfds);
			FD_SET(STDIN,&rfds);
			maxfd=max(RingInfo.A_fd,maxfd);
			maxfd=max(RingInfo.B_fd,maxfd);
			maxfd==max(maxfd,afd);
			//verify if fd is  valid
			if(RingInfo.A_fd>-1)FD_SET(RingInfo.A_fd,&rfds);
			if(RingInfo.B_fd>-1)FD_SET(RingInfo.B_fd,&rfds);
			if(afd>-1)FD_SET(RingInfo.B_fd,&rfds);
			counter=select(maxfd+1,&rfds,
							(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)&tv);
			if (counter==0){
				TimeOutCase();//TIMEOUT
			}
			if (counter<0) myerr(158,"Failed in appRun/Select");
			if (FD_ISSET(STDIN,&rfds)){
				StateCode=StateMachine(STDIN,SM_InStd);
			}
			if (FD_ISSET(udp_fp,&rfds)){
				StateCode=StateMachine(udp_fp,SM_InUdp);
			}
			if (FD_ISSET(tcp_fd,&rfds)){
				StateCode=StateMachine(tcp_fd,SM_InTcp);
			}
			if (FD_ISSET(RingInfo.B_fd,&rfds)){
				StateCode=StateMachine(RingInfo.B_fd,SM_InRing);
			}
			/*
			if (FD_ISSET(RingInfo.A_fd,&rfds)){
				//NEW TCP MSG
			}/**/
		}
	}
	
	void TimeOutCase(){
		//reset to Off StateCode
		if (AppState.state!=Off || AppState.state!=On)
			switch(AppState.state){
				case g_s_ok:ServX=-1;AppState.state=Off;break;
				case s_so_ds:ServX=-1;AppState.state=Off;break;
				case s_so_dso:AppState.state=w_s;break;
				case s_s_ok:ServX=-1;AppState.state=Off;break;
				case s_ds_ok:AppState.state=w_s;break;
				case w_s_ok:AppState.ss=false;ServX=-1;AppState.state=Off;break;
				case w_ds_ok:AppState.ds=false;ServX=-1;AppState.state=Off;break;
			}
	}
	int StateMachine(int fd,int fdType){
		int OldState=-1;
		
		//change machine state
		switch(fdType){
			case SM_InStd: //LocalUserInput
				if (SM_LocalUserInput(fd)){return SM_SysOff};
				break;
			case SM_InUdp:SM_UDP(fd);break;//client Only msg
			case SM_InTcp:SM_NewTCP(fd);break;
			case SM_InRing:SM_RingB();break;
			default: break;
		}
		//State Machine definition
		while (AppState.state!=OldState){
			OldState=AppState.state;
			switch(AppState.state){
				case g_s:if (get_start()){AppState.state=g_s_ok}else{AppState.state=Off};break;
				case g_s_ok:
					if (Ans_get_start())
						if(Oid==0 && Otpt==0 && 0==strcmp(Oip,"0.0.0.0")){
							if (set_start()){AppState.state=s_s_ds;break;}
						}else{
							Ring_SetA(Oip,Oid,Otpt);
							if(JoinRing(ServX,id,ip,tpt)>-1){
								AppState.ring=true;//Success
								AppState.state=On;
								break;
							}
							CleanRing();
						}
					ServX=-1;AppState.state=Off;break;
				case s_s_ds:if (set_start ()){AppState.state=s_so_ds;}else{AppState.state=Off}break;
				case s_so_ds:if(Ans_set_start()){AppState.ss=true;if(set_ds()){AppState.state=s_so_dso;break;}else{AppState.state=w_s;break}}AppState.state=Off;break;
				case s_so_dso:if(Ans_set_ds()){AppState.ds=true;AppState.state=On;}else{AppState.state=w_s;}break;
				case s_s:if (set_start ()){AppState.state=s_s_ok;}else{printfAppState.state=Off};break;
				case s_s_ok:if (Ans_set_start(){AppState.ss=true;AppState.state=On;}else{AppState.state=Off};break;
				case s_ds:if (set_ds()){AppState.ds=true;AppState.state=s_ds_ok;}else{AppState.state=Off};break;
				case s_ds_ok:if (Ans_set_ds()){AppState.state=On;}else{AppState.state=Off};break;
				case w_s:if(withdraw_start(ServX)){AppState.state=w_s_ok}else{AppState.state=Off;}break;
				case w_s_ok:if(Ans_withdraw_start()){AppState.ss=false;AppState.state=Off}break;//else{AppState.state=Off;}break;
				case w_ds:if(withdraw_ds(ServX)){AppState.state=w_ds_ok}else{AppState.state=Off;}break;	
				case w_ds_ok:if(Ans_withdraw_ds()){AppState.ds=false;AppState.state=Off}break;//else{AppState.state=Off;}break;
			}
		}
		return SM_SysOn;
	}
/*******************************/
/**** PrgmInputs            ****/
/*******************************/	
int SM_LocalUserInput(int fd){
	//reads user input from stdin and calls the correct function to execut user command
		int intaux;
		bufferclean();//Reset burffer data
		intaux=read(STDIN,myBuffer,buffersize-1);//prevent buffer overflow
		/*prevent loopbug*/
		if (intaux<=0) {getchar();return 0;}
		if myScmp("exit")){
			if (AppState.state==Off) return 1;//close prgm
		}else if(myScmp("join")){
			if (AppState.state==Off){
				//entrar no anel do serviço x
				// por omissao entrar no anel disponicel
				ServX=atoi(&myBuffer[4]);
				AppState.state=g_s;
				fprintf(stdout,">>Join ServX=%d\n",ServX);
			}else{fprintf(stdout,">>Leave (ServX=%d), before join anohter!\n",ServX);}
		}else if(myScmp("l")||myScmp("leave")){
			if (AppState.state!=Off){AppState.state=leave;}
			else {fprintf(stdout,">>Already OFF\n",ServX);}}
		}else if(myScmp("show_state")){
			printf(">ServerState:\n\tmyID:%i;\n\tServX:%i;\n\tstartS: %d@%s:%d\n\t%s (ss: %i  /  ds: %i / ring: %i)\n",id,ServX,
                   Oid,Oip,Otpt,
                   GetMState,AppState.ss,AppState.ds,AppState.ring);
			printf(">RingInfo-NodeA: (fd=%d) %d@%s:%d\n",RingInfo.A_fd,RingInfo.A_Id,RingInfo.A_IP,RingInfo.A_Port);
			printf(">RingInfo-NodeB: (fd=%d) %d@%s:%d\n",RingInfo.B_fd,RingInfo.B_Id,RingInfo.B_IP,RingInfo.B_Port);
		}else if myScmp("Force w_ss"){
			NotImpl;
		}else if myScmp("teste"){
			NotImpl;
		}else{
			fprintf(stdout,"Unknow Command!\n");
		}
		fflush(stdout);
		return 0;
}
void SM_RingB(){
	int intaux;
	struct sockaddr_in tpc_in_Addr;/*needed?*/
	#ifdef debug
		fprintf(stderr,"[INFO-appRun] NEW TCP MSG from previous Server (fd:%d)\n",RingInfo.B_fd);
	#endif
	intaux=0;//ini
	RingReadMSG();
	if(myScmp("NEW_START\n")){
		set_start(ServX);
	}else
		intaux=Ring(afd,&tpc_in_Addr,id,AppState.ss);//id means id of this server
	if (TOKEN=='S' && AppState.ds==false && AppState.state==On){
		set_ds(ServX);AppState.state=s_ds;
	}else if(TOKEN=='D'){
		set_ds(ServX);AppState.state=s_ds;
	}
	#ifdef debug
		fprintf(stderr,"[INFO-appRun] Token=%c Ring=%d %d %d \n",TOKEN,intaux,AppState.ds,false);
	#endif
	//conection error. close broken connections
	if(intaux==ErrRingB){
		//erro
		#ifdef debug
			fprintf(stderr,"[ERROR-appRun]close B_fd:%d\n",RingInfo.B_fd);
		#endif
		close(RingInfo.B_fd);RingInfo.B_fd=-1;
	}
	if(intaux==ErrRingA){
		//erro
		#ifdef debug
			fprintf(stderr,"[ERROR-appRun]close A_fd:%d\n",RingInfo.A_fd);
		#endif
		close(RingInfo.A_fd);RingInfo.A_fd=-1;
	}
}
void SM_NewTCP(int tcp_fd){
	int intaux;
	struct sockaddr_in tpc_in_Addr;
	//NEW TCP connection
	#ifdef debug
		fprintf(stderr,"[INFO-SM_NewTCP] NEW TCP Connection\n");
	#endif
	tpc_in_AddrSize=sizeof(tpc_in_Addr);
	listen(tcp_fd,myMaxTCP);
	afd=accept(tcp_fd,(struct sockaddr*)&tpc_in_Addr,&tpc_in_AddrSize);
	if (AppState.ss==false){close(afd);fprintf(stderr,"NotStart\n");return;}
	if (afd<0){
		//Error
		errnum=errno;
		#ifdef debug
			fprintf(stderr,"[INFO-SM_NewTCP] afd negative (%d)! %s\n",afd,strerror(errnum));
		#endif
	}else{
		if (0=<RingReadMsgFrom(afd)){return;}
		intaux=Ring(afd,&tpc_in_Addr,id,AppState.ss);//id means id of this server
		#ifdef debug
			fprintf(stderr,"[INFO-SM_NewTCP] afd=%d\t Ring:%d\n",afd,intaux);
		#endif
		if(intaux==ErrRingIngnore){
			//erro
			#ifdef debug
				fprintf(stderr,"[ERROR-SM_NewTCP] Close afd:%d\n",afd);
			#endif
			close(afd);
			afd=-1;
		}else{
			#ifdef debug
				if(AppState.ss)fprintf(stderr,"[INFO-SM_NewTCP] Ring Got New MEmber id:%d\n",RingInfo.B_Id);
			#endif
			//anel criado
			AppState.ring=1;
			AppState.state=On;
		}
	}
}

void SM_UDP(){
		if (-1==myrecv(udp_fp,NULL)){
			return;//do nothing
		}
		//clientRequests
		if (AppState.state!=On){
			if myScmp("MY_SERVICE ON\n"){
				//need to get it better organized
				C_addr=LastInAddr;//saves client addr
				if(-1==mySend("YOUR_SERVICE ON\n",udp_fp,C_addr)){
					return;//do nothing
				}
				if (AppState.ring){
					//in ring pass to next ds
					RingSetNodeBusy();
					withdraw_ds(ServX);
					bufferclean();//Reset burffer data
					sprintf(myBuffer,"TOKEN %d;S\n\0",id);//prevent buffer overflow
					RingMsgPidgeon(myBuffer);
					RingSetNodeBusy();
				}else{
					//alone
					printf(">>Busy on client (%s:%d)\n",inet_ntoa(C_addr.sin_addr),C_addr.sin_port);
					withdraw_ds(ServX);
				}
				return busy;
			}
			if myScmp("MY_SERVICE OFF\n"){
				//need to get it better organized
				if(-1==mySend("YOUR_SERVICE OFF\n",udp_fp,C_addr)){
					//fail to send
					return AppState.state;
				}
				if (AppState.ring){
					//in ring infor disponibility
					RingSetNodeIdle();
					bufferclean();//Reset burffer data
					if(RingBusy()){sprintf(myBuffer,"TOKEN %d;D\n\0",id);RingMsgPidgeon(myBuffer);}
					return ready;
				}else{
					//alone
					printf(">>Free client (%s:%d)\n",inet_ntoa(C_addr.sin_addr),C_addr.sin_port);
					set_ds(ServX);
					return s_ds;
				}
			}
		}
}
/*******************************/
/**** StatesInstructions    ****/
/*******************************/
