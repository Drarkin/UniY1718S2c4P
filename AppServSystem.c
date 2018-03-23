#include "AppServSystem.h"
#include <errno.h>
	int ReadyState(){
		if (-1==myrecv(udp_fp,NULL)){	
			return AppState.state;//do't change state
		}
		if myScmp("MY_SERVICE ON"){
			//need to get it better organized
			C_addr=LastInAddr;//saves client addr
			if(-1==mySend("YOUR_SERVICE ON\n",udp_fp,C_addr)){
				//fail to send
				return AppState.state;
			}
			if (AppState.ring){
				//in ring pass to next ds
			}else{
				//alone
				printf(">>Busy on client (%s:%d)\n",inet_ntoa(C_addr.sin_addr),C_addr.sin_port);
				withdraw_ds(ServX);
			}
			return busy;
		}else fprintf(stderr,">>Wrong MSG for ReadyState!\n");
		return AppState.state;//do't change state
	}
	int BusyState(){
		if (-1==myrecv(udp_fp,NULL)){	
			return AppState.state;//do't change state
		}
		if (LastInAddr.sin_addr.s_addr!=C_addr.sin_addr.s_addr){
			//wrong client address
			fprintf(stderr,">>Wrong Client (Addr):ignoring\n");
			return AppState.state;
		}
		if myScmp("MY_SERVICE OFF"){
			//need to get it better organized
			if(-1==mySend("YOUR_SERVICE OFF\n",udp_fp,C_addr)){
				//fail to send
				return AppState.state;
			}
			if (AppState.ring){
				//in ring pass to next ds
			}else{
				//alone
				printf(">>Free client (%s:%d)\n",inet_ntoa(C_addr.sin_addr),C_addr.sin_port);
				set_ds(ServX);
				return s_ds;
			fprintf(stderr,">>Wrong MSG for BusyState!\n");
			return busy;
			}
		}
		return AppState.state;//do't change state
	}

	int userIns(){
		//reads user input from stdin and calls the correct function to execut user command
		int intaux;
		bufferclean();//Reset burffer data
		intaux=read(STDIN,myBuffer,buffersize-1);//prevent buffer overflow
		/*prevent loopbug*/
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
            //Reset StartServerInfo O vars
            strcpy(Oip,"---.---.---.---");
            Oid=-1;
			Otpt=-1;
            ServX=-1;
            printf(">QUIT\n");
			AppState.ring=false;
			AppState.state=nready;
		}else if(myScmp("join")){
			if (AppState.state==nready){
				//entrar no anel do serviço x
				// por omissao entrar no anel disponicel
				ServX=atoi(&myBuffer[4]);
				fprintf(stderr,">>join with id %d\n",ServX);
				get_start(ServX);
			}else{
				fprintf(stderr,">>Can't Join Now! Already joined ServX:%d\n`\n\tUse \"Force w_ss\" to frocefull remove the service\n",ServX);
			}
		}else if(myScmp("show_state")){
			printf(">ServerState:\n\tmyID:%i;\n\tServX:%i;\n\tstartS: %d@%s:%d\n\t%s (ss: %i  /  ds: %i / ring: %i)\n",id,ServX,
                   Oid,Oip,Otpt,
                   GetMState,AppState.ss,AppState.ds,AppState.ring);
			//print state
		}else if (myScmp("leave")){
			//saida do servidor do anel
		}else if myScmp("set_ds"){
			fprintf(stderr,">>SET_DS!\n");
		}else if myScmp("Force w_ss"){
			fprintf(stderr,">>Force W_SS!\n");
			withdraw_start(ServX);
			ServX=-1;
		}else{
			fprintf(stdout,"Unknow Command!\n");
			fflush(stdout);
		}
		return 0;
	}
	void appRun(){
			int intaux;
			int errnum;
		//main code for service functionally
		//wait for new information to be read from any file descriptor and executs the correct answer for that input
		//enum {ini,idle,busy} state;//state that controls select
		int fd,newfd,afd;
		fd_set rfds;
		int maxfd,counter;
		struct sockaddr_in tpc_in_Addr;
		int tpc_in_AddrSize;
		//Find max value
		maxfd=max(tcp_fd,STDIN);
		maxfd=max(udp_fp,maxfd);
		CleanRing ();
		while(1){
			FD_ZERO(&rfds);
		#ifdef AppServRingVar
			FD_SET(tcp_fd,&rfds);
		#endif
			FD_SET(udp_fp,&rfds);
			FD_SET(STDIN,&rfds);
			//FD_SET((int)STDIN,&rfds);//jefc
		#ifdef AppServRingVar
			maxfd=max(RingInfo.A_fd,maxfd);
			maxfd=max(RingInfo.B_fd,maxfd);
			maxfd==max(maxfd,afd);
			//verify if fd is  valid
			
			if(RingInfo.A_fd>-1)FD_SET(RingInfo.A_fd,&rfds);
			if(RingInfo.B_fd>-1)FD_SET(RingInfo.B_fd,&rfds);
		#endif
			
			
			
			counter=select(maxfd+1,&rfds,
							(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);				
			if (counter<=0)myerr(158,"Failed in appRun()");
			
			//put code to read STDIN when input is writen
			if (FD_ISSET(STDIN,&rfds)){
				if(userIns())return;
			}	
		#ifdef AppServRingVar
			if (FD_ISSET(tcp_fd,&rfds)){
				//NEW TCP connection
				#ifdef debug
					fprintf(stderr,"[INFO-appRun] NEW TCP Connection\n");
				#endif
				tpc_in_AddrSize=sizeof(tpc_in_Addr);
				listen(tcp_fd,myMaxTCP);
				afd=accept(tcp_fd,(struct sockaddr*)&tpc_in_Addr,&tpc_in_AddrSize);
				if (afd<0){
					//Error
					errnum=errno;
					#ifdef debug
						fprintf(stderr,"[INFO-appRun] afd negative (%d)! %s\n",afd,strerror(errnum));
					#endif
					
				}else{
					intaux=Ring(afd,&tpc_in_Addr,id);
					#ifdef debug
						fprintf(stderr,"[INFO-appRun] afd=%d\t Ring=%d\n",afd,intaux);
					#endif
					if(intaux<0){
						//erro
						#ifdef debug
							fprintf(stderr,"[INFO-appRun] close afd:%d",afd);
						#endif
						close(afd);
					}
				}
			}
			if (FD_ISSET(RingInfo.B_fd,&rfds)){
				//NEW TCP Msg
				#ifdef debug
					fprintf(stderr,"[INFO-appRun] NEW TCP MSG from previous Server\n");
				#endif
				intaux=Ring(afd,&tpc_in_Addr,id);
				#ifdef debug
					fprintf(stderr,"[INFO-appRun] Ring=%d\n",intaux);
				#endif
			}
			/*
			if (FD_ISSET(RingInfo.A_fd,&rfds)){
				//NEW TCP MSG
			}/**/
		#endif
			if (FD_ISSET(udp_fp,&rfds)){
				switch (AppState.state){
					case g_s:
							if (Ans_get_start())AppState.state=g_s_ok;
							else AppState.state=nready;
 						break;
					case s_s: if (Ans_set_start()){
						AppState.state=s_s_ok;
						AppState.ss=true;
						}else AppState.state=nready;
						break;
					case s_ds: if (Ans_set_ds()){
							AppState.state=ready;
							AppState.ds=true;
						}else{
							AppState.state=nready;//w_s;//devido a erro remover from start server
							withdraw_start(ServX);
						}
						break;
					case ready:
						AppState.state=ReadyState();
						break;
					case busy:
						AppState.state=BusyState();
						break;
					default: break;	
				}
			}
			switch(AppState.state){
				case g_s_ok:
					if (Oid==0){
						set_start(ServX);//para o servico X
					}else{
						AppState.ring=true;
						AppState.state=joinR;
						serv_start();
						Ring_SetA(Oip,Oid,Otpt);
						JoinRing(ServX,id,ip,tpt);
						//set next ring address
						//conect to ring
					}
					break;
				case s_s_ok: //caso de 
					//next ring element is himself
					Oid=id;
					strcpy(Oip,ip);
					Otpt=tpt;
					//
					set_ds(ServX);
					AppState.state=s_ds;
					break;
				default: break;	
			}
		}
	}
