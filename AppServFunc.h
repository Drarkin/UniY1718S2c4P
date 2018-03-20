#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>	

#include "GeralCom.h"
#include "AppServVar.h"

	void myfpClose();
//CentralServer Comamnds
	void set_ds(int x);
	int Ans_set_ds();
	void withdraw_ds(int x);
	void set_start (int x);
	int Ans_set_start();
	void withdraw_start(int x);
	void get_start(int x);
	int Ans_get_start();
	void serv_start();
//Exit & Error
	void myerr(int code,char* msg);
	void myfpClose();
//Start SetUp
	void appConfg ( int argc,char **argv);