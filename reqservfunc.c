#include "AppServFunc.h"
	
	
//CentralServer Comamnds
	void my_service_ON(){
		sprintf(myBuffer,"MY SERVICE  ON\n");
		mysend(udp_fp,SC_addr);
	}
	
	void my_service_OFF(){
		sprintf(myBuffer,"MY SERVICE  OFF\n");
		mysend(udp_fp,SC_addr);
	}
	
	void your_service_ON(){
		sprintf(myBuffer,"YOUR SERVICE  ON\n");
		mysend(udp_fp,SC_addr);
	}
	
	void your_service_OFF(){
		sprintf(myBuffer,"YOUR SERVICE  OFF\n");
		mysend(udp_fp,SC_addr);
	}
	int Ans_get_ds_server(){
		int n;
		if(-1==myrecv(udp_fp,SC_addr)){
			return 0;
		}
		if (0!=sscanf(myBuffer,"OK %i;%s:%i",&Oid,Oip,&Otpt) && 0!=Oid && 0!=Otpt) {
			fprintf(stderr,">>Ok!\n);
			return 1;
		}
		fprintf(stderr,">>Err WrongMsg\n");
		return 0;
	}
	void get_ds_server(int x){
		sprintf(myBuffer,"GET_DS_SERVER %d\n"),x;
		mysend(udp_fp,SC_addr);
	}
	int Ans_get_ds_server(){
		int n;
		if(-1==myrecv(udp_fp,SC_addr)){
			return 0;
		}
		if (0!=sscanf(myBuffer,"OK %i;%s:%i",&Oid,Oip,&Otpt) && 0!=Oid && 0!=Otpt) {
			fprintf(stderr,">>Ok!\n);
			return 1;
		}
		fprintf(stderr,">>Err WrongMsg\n");
		return 0;
	}
	
	//Exit & Eror
	void myerr(int code,char* msg){
		fprintf(stderr,"Err_%d",code);
		if (msg!=NULL) fprintf(stderr,":%s",msg);
		fprintf(stderr,"\n");
		myfpClose();
		exit(-code);
	}


	void myfpClose(){
		if (udp_fp>=0){ close(udp_fp);udp_fp=-1;}
		if (tcp_fd>=0){ close(tcp_fd);tcp_fd=-1;}
		return;
	}	
	
//userinput
	int userIns(){
		//reads user input from stdin and calls the correct function to execut user command
		int intaux;
		myBuffer[0]='\0';//Reset burffer data
		intaux=scanf("%[^\n]",myBuffer);
		/*prevent loopbug*/
		if (intaux<=0) {getchar();return 0;}
		if (myScmp("quit")||myScmp("exit")){
			if (state==false){
				return 0;
			}else printf("Terminate service first!\n");
		}else if(myScmp("request_service")||myScmp("rs ")){
			if (sscanf(" %d",&ServX)){
				get_ds_server(ServX);
			}else printf("Wrong input!\n");
			
		}else if(myScmp("terminate_service")||myScmp("ts")){
			if (state){
				my_service_OFF();
			}else printf ("No service running\n");
		}else{
			fprintf(stdout,"Unknow Command!\n");
			fflush(stdout);
		}
		return 1;
	}
	int ReqServMachine(){
		enum {ini,idle,busy} state;//state that controls select
		int fd,newfd,afd;
		fd_set rfds;
		int maxfd,counter;
		//Find max value
		maxfd=(udp_fp>STDIN)?udp_fp:STDIN;
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
				return userIns();
			}
			if (FD_ISSET(udp_fp,&rfds)){
				
			}
	return 1;
	}
//Start SetUp

void appConfg ( int argc,char **argv){
	//arg is a vector of string,
	//arg last pointer must be NULL
	int i=0;
	char *SName=DV_csName;//CentralServer Name
	cspt=DV_cspt;
	int argCheck = 0;
	while (argv[i]!=NULL){
		if (strcmp(argv[i],"-i")==0 && argc>i){
			i++;
			SName=argv[i];
		}else	if (strcmp(argv[i],"-p")==0 && argc>i){
			i++;
			cspt=atoi(argv[i]);
		}
		i++;
	}
	//check arguments for service
	//get CentralServer IP
	csh = gethostbyname(SName);
	if (csh==NULL) myerr(2,"Fail to gethostbyname");
	csa=(struct in_addr*)csh->h_addr_list[0];
	//Show Info
	fprintf(stderr,"Service:\n\tid: %d\n\tip: %s\n\tupt: %d\n\ttpt: %d\n",id,ip,upt,tpt);
	fprintf(stderr,"\nCentralServer:\n\t<%s:%d>\n",SName,cspt);
	fprintf(stderr,"\tofficial host name: %s\n",csh->h_name);
	fprintf(stderr,"\tinternet address: %s (%08lX)\n",inet_ntoa(*csa),(long unsigned int)ntohl(csa->s_addr));
	return;
	}