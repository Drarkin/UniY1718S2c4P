#include "reqservVar.h"
#include "GeralCom.h"

#include <unistd.h>
#include <string.h>

void my_service_ON();
int Ans_myON();
void my_service_OFF();
int Ans_myOFF();
void your_service_ON();
void your_service_OFF();
void get_ds_server(int x);	
int Ans_get_ds_server();
void myerr(int code,char* msg);
void myfpClose();
int userIns();
int ReqServApp();
void appConfg ( int argc,char **argv);