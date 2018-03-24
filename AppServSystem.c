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
				RingSetNodeBusy();
				withdraw_ds(ServX);
				bufferclean();//Reset burffer data
				sprintf(myBuffer,"TOKEN %d;S\n\0",id);//prevent buffer overflow
				RingMsgPidgeon(myBuffer);
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
				//in ring infor disponibility
				RingSetNodeIdle();
				bufferclean();//Reset burffer data
				sprintf(myBuffer,"TOKEN %d;D\n\0",id);//prevent buffer overflow
				RingMsgPidgeon(myBuffer);
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
	void AppReset(){
		//Reset StartServerInfo O vars
            strcpy(Oip,"---.---.---.---");
            Oid=-1;
			Otpt=-1;
            ServX=-1;
            printf(">QUIT\n");
			AppState.ring=false;
			AppState.state=nready;
	}
	int userIns(){
		//reads user input from stdin and calls the correct function to execut user command
		int intaux;
		bufferclean();//Reset burffer data
		intaux=read(STDIN,myBuffer,buffersize-1);//prevent buffer overflow
		/*prevent loopbug*/
		if (intaux<=0) {getchar();return 0;}
		if (myScmp("q")||myScmp("quit")||myScmp("exit")){
			if myScmp("exit")
				if (!(AppState.ds || AppState.ss || AppState.ring))return 1;
				else fprintf(stderr,"still conectect!\n");
			if(AppState.ring==0){
				if (AppState.ds==true){
					withdraw_ds(ServX);
				}
				if (AppState.ss==true){
					withdraw_start(ServX);
					#ifdef AppServRingVar
						RingMsgPidgeon("NEW START\n\0");
					#endif
				}
				AppReset();
			}else fprintf(stderr,"[WARNING-userIns] LEave the ring first!\n");

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
		}else if(myScmp("l")||myScmp("leave")){
			#ifdef AppServRingVar
			fprintf(stderr,">START LEAVE!\n");
			if (AppState.ss){
				withdraw_start(ServX);
				bufferclean();//Reset burffer data
				sprintf(myBuffer,"NEW_START\n\0");//prevent buffer overflow
				RingMsgPidgeon(myBuffer);
			}
			if (AppState.ds){
				withdraw_ds(ServX);
				bufferclean();//Reset burffer data
				sprintf(myBuffer,"TOKEN %d;I\n\0",id);//prevent buffer overflow
				RingMsgPidgeon(myBuffer);
			}
			if (AppState.ring){
				bufferclean();//Reset burffer data
				sprintf(myBuffer,"TOKEN %d;O;%d;%s%d;\n\0",id,RingInfo.A_Id,RingInfo.A_IP,RingInfo.A_Port);//prevent buffer overflow
				RingMsgPidgeon(myBuffer);
				//clean state to ini
				close(RingInfo.A_fd);RingInfo.A_fd=-1;
				close(RingInfo.B_fd);RingInfo.B_fd=-1;
				CleanRing ();
			}
			AppReset();
			#endif


		}else if(myScmp("show_state")){
			printf(">ServerState:\n\tmyID:%i;\n\tServX:%i;\n\tstartS: %d@%s:%d\n\t%s (ss: %i  /  ds: %i / ring: %i)\n",id,ServX,
                   Oid,Oip,Otpt,
                   GetMState,AppState.ss,AppState.ds,AppState.ring);
		#ifdef AppServRingVar
			printf(">RingInfo-NodeA: (fd=%d) %d@%s:%d\n",RingInfo.A_fd,RingInfo.A_Id,RingInfo.A_IP,RingInfo.A_Port);
			printf(">RingInfo-NodeB: (fd=%d) %d@%s:%d\n",RingInfo.B_fd,RingInfo.B_Id,RingInfo.B_IP,RingInfo.B_Port);
		#endif
			//print state
		}else if myScmp("Force w_ss"){
			fprintf(stderr,">>Force W_SS!\n");
			withdraw_start(ServX);
			ServX=-1;
		}else if myScmp("teste"){
			#ifdef AppServRingVar
				bufferclean();//Reset burffer data
				sprintf(myBuffer,"TOKEN %d;M\n\0",id);//prevent buffer overflow
				RingMsgPidgeon(myBuffer);
			#endif
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
			if(afd>-1)FD_SET(RingInfo.B_fd,&rfds);
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
					intaux=Ring(afd,&tpc_in_Addr,id,AppState.ss);//id means id of this server
					#ifdef debug
						fprintf(stderr,"[INFO-appRun] afd=%d\t Ring:%d\n",afd,intaux);
					#endif
					if(intaux==ErrRingIngnore){
						//erro
						#ifdef debug
							fprintf(stderr,"[ERROR-appRun] Close afd:%d\n",afd);
						#endif
						close(afd);
						afd=-1;
					}else{
						//erro
						#ifdef debug
							if(AppState.ss)fprintf(stderr,"[INFO-appRun] Ring Got New MEmber id:%d\n",RingInfo.B_Id);
						#endif
						//anel criado
						AppState.ring=1;
						AppState.state=ready;
					}
				}
			}
			if (FD_ISSET(RingInfo.B_fd,&rfds)){
				//NEW TCP Msg
				#ifdef debug
					fprintf(stderr,"[INFO-appRun] NEW TCP MSG from previous Server (fd:%d)\n",RingInfo.B_fd);
				#endif
				intaux=0;//ini
				RingReadMSG();
				if(myScmp("NEW_START\n")){
					set_start(ServX);
				}else
					intaux=Ring(afd,&tpc_in_Addr,id,AppState.ss);//id means id of this server
				if (TOKEN=='S' && AppState.ds==false && AppState.state==ready){
					fprintf(stderr,"REMOVER!!! fazer set ds\n");
					set_ds(ServX);
					AppState.state=s_ds;
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
					close(RingInfo.B_fd);
					RingInfo.B_fd=-1;
				}
				if(intaux==ErrRingA){
					//erro
					#ifdef debug
						fprintf(stderr,"[ERROR-appRun]close A_fd:%d\n",RingInfo.A_fd);
					#endif
					close(RingInfo.A_fd);
					RingInfo.A_fd=-1;
				}
			}
			/*
			if (FD_ISSET(RingInfo.A_fd,&rfds)){
				//NEW TCP MSG
			}/**/
		#endif//
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
						AppState.state=joinR;
						serv_start();
						Ring_SetA(Oip,Oid,Otpt);
						if(JoinRing(ServX,id,ip,tpt)>-1){
							//Success
							AppState.ring=true;
						}
						//fail to joinring
					}
					break;
				case s_s_ok: //caso de
					//next ring element is himself
					if(AppState.ring) {
						AppState.state=ready;

					}else{
						Oid=id;
						strcpy(Oip,ip);
						Otpt=tpt;
						//
						set_ds(ServX);
						AppState.state=s_ds;
					}
					break;
				default: break;
			}
			//case all conections to ring are broken
			if(RingInfo.A_fd==RingInfo.B_fd){
					AppState.ring=0;
					RingInfo.type=uno;
					CleanRing();
			}
		}
	}
