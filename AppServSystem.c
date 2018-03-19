#include "AppServSystem.h"
	int userIns(){
		//reads user input from stdin and calls the correct function to execut user command
		int intaux;
		myBuffer[0]='\0';//Reset burffer data
		intaux=scanf("%[^\n]",myBuffer);
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
		}else if(myScmp("join")){
			//entrar no anel do serviço x
			// por omissao entrar no anel disponicel
			ServX=atoi(&myBuffer[4]);
			fprintf(stderr,">>join with id %d\n",ServX);
			get_start(ServX);
		}else if(myScmp("show_state")){
			printf("\tServerState(%i{Serv:%i}): %i (ss: %i  /  ds: %i)\n",id,ServX,AppState.state,AppState.ss,AppState.ds);
			//print state
		}else if (myScmp("leave")){
			//saida do servidor do anel
		}else if myScmp("set_ds"){
			fprintf(stderr,">>SET_DS!\n");
		}else{
			fprintf(stdout,"Unknow Command!\n");
			fflush(stdout);
		}
		return 0;
	}
	
		void appRun(){
		//main code for service functionally
		//wait for new information to be read from any file descriptor and executs the correct answer for that input
		enum {ini,idle,busy} state;//state that controls select
		int fd,newfd,afd;
		fd_set rfds;
		int maxfd,counter;
		//Find max value
		maxfd=(tcp_fd>STDIN)?tcp_fd:STDIN;
		maxfd=(udp_fp>maxfd)?udp_fp:maxfd;
		while(1){
			FD_ZERO(&rfds);
			FD_SET(tcp_fd,&rfds);
			FD_SET(udp_fp,&rfds);
			FD_SET(STDIN,&rfds);
			//FD_SET((int)STDIN,&rfds);//jefc
			if(state==busy){FD_SET(afd,&rfds);maxfd=max(maxfd,afd);}
			
			counter=select(maxfd+1,&rfds,
							(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);				
			if (counter<=0)myerr(158,"Failed in appRun()");
			
			//put code to read STDIN when input is writen
			if (FD_ISSET(STDIN,&rfds)){
				if(userIns())return;
			}
			if (FD_ISSET(tcp_fd,&rfds)){
			}
			if (FD_ISSET(udp_fp,&rfds)){
				switch (AppState.state){
					case g_s:
							if (Ans_get_start())AppState.state=g_s_ok;
							else AppState.state=nready;
 						break;
					case s_s: Ans_set_start();
						break;
					case s_ds: Ans_set_ds();
							AppState.state=ready;
					default: break;	
				}
			}
			switch(AppState.state){
				case g_s_ok:
					if (Oid==0){
						set_start(ServX);//para o servico X
					}else{
						AppState.state=ready;
						serv_start();
						//set next ring address
						//conect to ring
					}
					break;
				case s_s_ok: //caso de 
					//next ring element is himself
					Oid=id;
					strcpy(Oip,ip);
					Otpt=tpt;
					set_ds(ServX);
					AppState.state=s_ds;
					break;
				default: break;	
			}
		}
	}