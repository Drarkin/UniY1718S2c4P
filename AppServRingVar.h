/**MADE BY DRARKIN**/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#define GetType 	GetTypeVectorName[RingInfo.type] //indica o tipo do anel
#define MYIPSIZE 16
#define AppServRingVar
#define debug
extern const char *GetTypeVectorName[];
 
extern struct RingInfoType{	
	enum{uno,duo,mul,halfway} type;//diz o numero de membros do anel. half means that the ring is passing from uno to duo
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

//Before using JoinRing, The address of StartServer should already be written into A zone of RingInfo by using Ring_SetA
void CleanRing ();
void Ring_SetA(char *A_IP,int A_Id,int A_Port);
void Ring_SetB(char *B_IP,int B_Id,int B_Port);
int Ring(int fd2read,struct sockaddr_in *addr,int myId);
int OuroborosHead(int myId);
int OuroborosTail(int B_fd,struct sockaddr_in *B_addr,int myId);
int JoinRing(int ServId,int myId,char *myIP,int myPort);
int CreateRing(int tcp_fdB,struct sockaddr_in *addr,int myId);
   
