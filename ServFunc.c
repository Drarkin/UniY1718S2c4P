#include "ServFunc.h"

int get_start(){
	sprintf(myBuffer,"GET_START %i;%i\n",x,id);
	if (mysend(udp_fp,SC_addr)==strlen(myBuffer))
		return 1;
	return 0;
}