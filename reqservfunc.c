#include "reqservfunc.h"	
	
//CentralServer Comamnds
	void my_service_ON(){
		sprintf(myBuffer,"MY_SERVICE ON\n");
		mysend(udp_fp,S_addr);
	}
	
	int Ans_myON(){
		int n;
		if(-1==myrecv(udp_fp,&S_addr)){
			return 0;
		}
		if (myScmp("YOUR_SERVICE ON")) {
			fprintf(stderr,">>Ok!\n");
			return 1;
		}
		fprintf(stderr,">>Err WrongMsg\n");
		return 0;
	}
	
	void my_service_OFF(){
		sprintf(myBuffer,"MY_SERVICE OFF\n");
		mysend(udp_fp,S_addr);
	}
	int Ans_myOFF(){
		int n;
		if(-1==myrecv(udp_fp,&S_addr)){
			return 0;
		}
		if (myScmp("YOUR_SERVICE OFF")) {
			fprintf(stderr,">>Ok!\n");
			return 1;
		}
		fprintf(stderr,">>Err WrongMsg\n");
		return 0;
	}
	
	void your_service_ON(){
		sprintf(myBuffer,"YOUR_SERVICE ON\n");
		mysend(udp_fp,S_addr);
	}
	
	void your_service_OFF(){
		sprintf(myBuffer,"YOUR_SERVICE OFF\n");
		mysend(udp_fp,SC_addr);
	}
	void get_ds_server(int x){
		sprintf(myBuffer,"GET_DS_SERVER %d\n",x);
		mysend(udp_fp,SC_addr);
	}
	int Ans_get_ds_server(){
		int n;
        //Set old values to zero
        Oid=-1;Otpt=-1;strcpy(Oip,"---.---.---.---");
		if(-1==myrecv(udp_fp,&SC_addr)){
			return 0;
		}
		if (myScmp("OK 0;0.0.0.0;0")){
			printf(">>No Service Server Avaiable!\n");
			return 0;
		}
		if (3==sscanf(myBuffer,"OK %i;%[^;];%i",&Oid,Oip,&Otpt)) {
			fprintf(stderr,">>Ok! (%i %s %i )\n",Oid,Oip,Otpt);
			return 1;
		}
		fprintf(stderr,">>Err WrongMsg! (%i %s %i )\n",Oid,Oip,Otpt);
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
		return;
	}	
	
//userinput
	int userIns(){
		//reads user input from stdin and calls the correct function to execut user command
		int intaux;
		bufferclean();//Reset burffer data
		intaux=read(STDIN,myBuffer,buffersize-1);//prevent buffer overflow
		/*prevent loopbug*/
		if (intaux<=0) {getchar();return 0;}
		if (myScmp("quit")||myScmp("exit")){
			if ( state.service==off && state.state==zero){
				return 1;
			}else printf("Terminate service first!\n");
		//
		}else if(myScmp("request_service")){
			if (state.service==off){
				if (sscanf(&myBuffer[15],"%d",&ServX)){
					get_ds_server(ServX);
					state.state=get;
				}else printf("Wrong input!\n");
			}else{
				printf("Service(%i) is Running\n",ServX);
			}
		}else if(myScmp("rs ")){
			if (state.service==off){
				if (sscanf(&myBuffer[2],"%d",&ServX)){
					get_ds_server(ServX);
					state.state=get;
				}else printf("Wrong input!\n");
			}else{
				printf("Service(%i) is Running\n",ServX);
			}
		}else if(myScmp("terminate_service")||myScmp("ts")){
			if (state.service){
				my_service_OFF();
				state.state=myOFF;
			}else printf ("No service running\n");
		}else if(myScmp("help")||myScmp("h")){
			printf ("\trequest_service (rs) <number>\n\tterminate_service (ts)\n");
		}else{
			fprintf(stdout,"Unknow Command!\n");
			fflush(stdout);
		}
		return 0;
	}
	int ReqServApp(){
		enum {ini,idle,busy} State;//state that controls select
		int fd,newfd,afd;
		fd_set rfds;
		int maxfd,counter;
		//Find max value
		maxfd=(udp_fp>STDIN)?udp_fp:STDIN;
		while(1){
			FD_ZERO(&rfds);
			FD_SET(udp_fp,&rfds);
			FD_SET(STDIN,&rfds);
			//FD_SET((int)STDIN,&rfds);//jefc
			if(State==busy){FD_SET(afd,&rfds);maxfd=max(maxfd,afd);}
			
			counter=select(maxfd+1,&rfds,
							(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);				
			if (counter<=0)myerr(158,"Failed in appRun()");
			
			//put code to read STDIN when input is writen
			if (FD_ISSET(STDIN,&rfds)){
				if (userIns())return 0;
			}
			if (FD_ISSET(udp_fp,&rfds)){
				switch (state.state){
					case get: if(Ans_get_ds_server()){
							//service address
							S_addr.sin_addr.s_addr=inet_addr(Oip);
							S_addr.sin_family=AF_INET;
							S_addr.sin_port=htons(Otpt); 
							my_service_ON(),
							state.state=myON;
						}else {
							//printf("Failed: GET DS Server\n");
							state.state=zero;//back to standby
						}
						break;
					case myON: if(Ans_myON()){
							printf("Service is ON\n");
							state.service=on;
							state.state=zero;						
						}else{
							state.state=zero;//back to standby
						}
						break;
					case myOFF: if (Ans_myOFF()){
							printf (">>Service OFF\n");
							state.service=off;
							state.state=zero;
						}else{
							state.state=zero;//back to standby
						}
						break;
					default:break;
				}
				
			}
		}
	return -1;
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
	fprintf(stderr,"RequestService\n");
	fprintf(stderr,"\nCentralServer:\n\t<%s:%d>\n",SName,cspt);
	fprintf(stderr,"\tofficial host name: %s\n",csh->h_name);
	fprintf(stderr,"\tinternet address: %s (%08lX)\n",inet_ntoa(*csa),(long unsigned int)ntohl(csa->s_addr));
	return;
	}
