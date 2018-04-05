#include "ServFunc.h"

int get_start(){
	sprintf(myBuffer,"GET_START %i;%i\n",x,id);
	if (mysend(udp_fp,SC_addr)==strlen(myBuffer))
		return 1;
	return 0;
}

int Ans_get_start(){
	int n=-1;
	int ip1,ip2,ip3,ip4;
	//clean vars
	Oid=-1;
	strcpy(Oip,"---.---.---.---");
	Otpt=-1;
	if (-1==myrecv(udp_fp,NULL)){			
		return 0;
	}
	if (0!=sscanf(myBuffer,"OK %i;%i;%d.%d.%d.%d;%i\n",&n,&Oid,Oip,&ip1,&ip2,&ip3,&ip4,&Otpt) && n==id){
		if(ip1<=255 && ip2<=255 && ip3<=255 && ip4<=255) sprintf(Oip,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);else return 0;
		fprintf(stderr,">>Ok!\t(myid: %i ;id2: %i ;ip2: %s ;tpt2: %i)\n",n,Oid,Oip,Otpt);
		if(n==Oid){
			fprintf(stderr,"Err! Exists already a ServiceServer with same id\n");
			return 0;
		}
		return 1;
	}
	fprintf(stderr,">>Err WrongMsg! (%d %d %s %d)\n",Oid,n,Oip,Otpt);
	return 0;
}


int set_start (){
	sprintf(myBuffer,"SET_START %i;%i;%s;%i\n",ServX,id,ip,tpt);//verify tpt
	if (mysend(udp_fp,SC_addr))
		return 1;
	return 0;
}
int Ans_set_start(){
	int n=-1;
	if(-1==myrecv(udp_fp,NULL)){
		return 0;
	}
	if (0!=sscanf(myBuffer,"OK %i;",&n) && n==id) {
		fprintf(stderr,">>Ok!\t(id: %d ;)\n",n);
		return 1;
	}
	fprintf(stderr,">>Err WrongMsg ( %d )\n",n);
	return 0;
}


int set_ds(){
		sprintf(myBuffer,"SET_DS %i;%i;%s;%i\n",ServX,id,ip,upt);
		if(mysend(udp_fp,SC_addr))
			return 1;
		return 0;
	}
int Ans_set_ds(){
	int n=-1;
	if(-1==myrecv(udp_fp,NULL)){
		return 0;
	}
	if (0!=sscanf(myBuffer,"OK %i;",&n) && n==id) {
		fprintf(stderr,">>Ok!\t(id: %i )\n",n);
		return 1;
	}
	fprintf(stderr,">>Err WrongMsg ( %d )\n",n);
	return 0;
}


	int withdraw_start(int x){
		sprintf(myBuffer,"WITHDRAW_START %i;%i\n",x,id);
		mysend(udp_fp,SC_addr);
		myrecv(udp_fp,&SC_addr);//set timmeout
	}
	int Ans_withdraw_start(){
		if(-1==myrecv(udp_fp,NULL)){
			return 0;
		}
		if (0!=sscanf(myBuffer,"OK %i;",&n) && n==id) {
			fprintf(stderr,">>Ok!\t(id: %i )\n",n);
			return 1;
		}
		fprintf(stderr,">>Err WrongMsg ( %d )\n",n);
		return 0;
	}
	
	int withdraw_ds(int x){
		sprintf(myBuffer,"WITHDRAW_DS %i;%i\n",x,id);
		if(mysend(udp_fp,SC_addr))
			return 1;
		return 0;
	}	
	int Ans_withdraw_ds(){
		if(-1==myrecv(udp_fp,NULL)){
			return 0;
		}
		if (0!=sscanf(myBuffer,"OK %i;",&n) && n==id) {
			fprintf(stderr,">>Ok!\t(id: %i )\n",n);
			return 1;
		}
		fprintf(stderr,">>Err WrongMsg ( %d )\n",n);
		return 0;
	}	
/*******************************/
/**** Serv2ReqServ          ****/
/*******************************/
	void your_service_ON(struct sockaddr_in addr){
		mySend("YOUR SERVICE  ON\n",udp_fp,SC_addr);
	}
	
	void your_service_OFF(struct sockaddr_in addr){
		mySend("YOUR SERVICE  OFF\n",udp_fp,SC_addr);
	}
/*******************************/
/**** PrgmIniConfig         ****/
/*******************************/	
void appConfg ( int argc,char **argv){
	//arg is a vector of string,
	//arg last pointer must be NULL
	int i=0;
	char *SName=DV_csName;//CentralServer Name
	cspt=DV_cspt;
	int argCheck = 0;
	while (argv[i]!=NULL){
		if (strcmp(argv[i],"-n")==0 && argc>i){
			i++;
			argCheck++;
			id=atoi(argv[i]);
		}else	if (strcmp(argv[i],"-j")==0 && argc>i){
			i++;
			argCheck++;
			ip=argv[i];
		}else   if (strcmp(argv[i],"-u")==0 && argc>i){
                        i++;
			argCheck++;
			upt=atoi(argv[i]);
                }else   if (strcmp(argv[i],"-t")==0 && argc>i){
                        i++;
			argCheck++;
			tpt=atoi(argv[i]);
                }
		if (strcmp(argv[i],"-i")==0 && argc>i){
			i++;
			SName=argv[i];
		}else	if (strcmp(argv[i],"-p")==0 && argc>i){
			i++;
			cspt=atoi(argv[i]);
		}
		i++;
	}
    //reset O var

    strcpy(Oip,"---.---.---.---");
    Oid=-1;
	Otpt=-1;
    
    ServX=-1;
	//check arguments for service
	if(argCheck!=4)myerr(1,"missing arguments");
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