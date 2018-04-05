/**MADE BY DRARKIN**/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#define GetType 	GetTypeVectorName[RingInfo.type] //indica o tipo do anel
#define MYIPSIZE 16
/**
	Este codigo serve só para a criação do anel e entrada e saida dos seus membros, bem como a passagem de Tokens respectivos a gestao de membros do anel
	funcoes principais
		RingMsgPidgeon - Enviar mensagens entre os membros do anele
		
		Ring - analisar msg do anel e executar as operaçoes necessarias para a entrada de membros e saida
		
**/

#define RingMsgSize_MAX 100//72
#define AppServRingVar
#define debug
#define ErrRingIngnore -1
#define ErrRingA -3
#define ErrRingB -2
#define ErrRingNewfd -4
#define ErrRingNoRing -5
extern const char *GetTypeVectorName[];
extern char RingMsgBuffer[RingMsgSize_MAX];
extern char TOKEN;
extern struct RingInfoType{	
	enum{uno,halfway,duo,mul} type;//diz o numero de membros do anel. half means that the ring is passing from uno to duo
	int B_fd;
	struct sockaddr_in B_addr;
	char B_IP[MYIPSIZE];
	int B_Id;
	int B_Port; //max value of 65535`
	int A_fd;
	struct sockaddr_in A_addr;
	char A_IP[MYIPSIZE];
	int A_Id;
	int A_Port; //max value of 65535
	} RingInfo;
  
  /** NOTAS          
		   struct sockaddr_in {
               sa_family_t    sin_family; // address family: AF_INET 
               in_port_t      sin_port;   // port in network byte order 
               struct in_addr sin_addr;   // internet address 
           };

           // Internet address. 
           struct in_addr {
               uint32_t       s_addr;     // address in network byte order 
           };
		   */

//setting state of node/ring
/*<->*/ int RingOn();
/*<->*/ int RingBusy();
/*<->*/ int RingNodeBusy();
/*<->*/void RingSetNodeIdle();
/*<->*/void RingSetNodeBusy();
/*<->*/void RingSetBusy();
/*<->*/void RingSetIdle();
//Ring Creation
/*<1>*/char *RingReadMSG();//Reads Messages from the previous (antecent) Node
/*<2>*/int Ring(int fd2read,struct sockaddr_in *addr,int myId,int StartServer);//state machines that manages ring
/*<->*/int JoinRing(int ServId,int myId,char *myIP,int myPort);//process of entering in an existing ring 
/*NOTE** //Before using JoinRing, The address of StartServer should already be written into A zone of RingInfo by using Ring_SetA */
/*<->*/int RingMsgPidgeon(char *msg);//sends msg around the ring
/*<->*/void CleanRing ();//Deletes all information regarding the connected nodes.
/*<->*/void Ring_SetA(char *A_IP,int A_Id,int A_Port);//set data about sucessor adrress and fd
/*<->*/void Ring_SetB(char *B_IP,int B_Id,int B_Port);//set data about antecent address and fd

/** Not Use Outside AppServRing **/
int OuroborosHead(int myId);//dragon's head that bites the tail
int OuroborosTail(int B_fd,struct sockaddr_in *B_addr,int myId);//new dragon's tail, it will be bitten
int CreateRing(int tcp_fdB,struct sockaddr_in *addr,int myId);//Creates the Ring when a second server tries to conect
int NewServer(int tcp_fdB,int myID);//Adds a new server into the Ring 
int RingToken(int myId);//State Machine that manages tokens circulation
   
