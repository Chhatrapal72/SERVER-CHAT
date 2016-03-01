/**
 * @file       : servcht_client.c 
 * @brief      :  client sends the message to server and recieve message of another client
 *		  from the server                

 */

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<errno.h>
#define MAX 100
#define C_Len 15
#define REGISTRATION 1
#define LOGOUT 2
#define DUPLICATE 3
#define PING 4
#define ALREADYR 5
/*
 * @struct	:MSG
 * @brief	:structure which hold client name and message which will pass to the server
 * @member	:
 *	@mem1	:client name
 *	@mem2	:message
 * 	@mem3	:identify the purpose of client 
*/
typedef struct{
	char C_name[C_Len];
	char msg[MAX];
	int identifier;
}MSG;
/*
 * @struct      :sock_serv_detail
 * @brief       :structure which hold socket,ip address of server and port number which will use to send messages to the server
 * @member      :
 *      @mem1   :socket number
 *      @mem2   :ip address of server
 *	@mem3	:port number of server
*/
typedef struct{
	int sockfd;
	char *ip;
	char *port;
}sock_serv_detail;

MSG *Registration=NULL;
char user_name[C_Len];
char C_name[C_Len];
void Register_at_server(int,struct sockaddr_in );
void *send_msg(void*);
void *recieve_msg(void*);
int flag=0;
int identifier=0;
/*
 * @function    : main 
 * @params      : receives command line arguments
 * @return      : none 
 * @brief       : client main
 */

int main(int argc,char *argv[])
{
	int sockfd,msglen,len=0,res,try;
	socklen_t serv_addr_size;
	struct sockaddr_in servaddr,clientaddr;
	sock_serv_detail detail;
	void *thk;
	MSG msg;
	char message[MAX];
	pthread_t sendthread,recvthread;
	struct timeval timeout;	
	/* Create socket  for recieving and sending messages*/
	if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0)
	{
		fprintf(stdout,"Socket creation fail:[%s]\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(stdout,"\nSocket successfully Created.\n");

	memset((&servaddr),0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(atoi(argv[2]));
	servaddr.sin_addr.s_addr=inet_addr(argv[1]);
	
	/*Create name socket for recieving messages*/
	memset((&clientaddr),0,sizeof(clientaddr));
	clientaddr.sin_family=AF_INET;
        clientaddr.sin_port=htons(atoi(argv[2]));
        clientaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    	
	if(bind(sockfd,(struct sockaddr *)&clientaddr,sizeof(clientaddr))<0)
	{
		
		fprintf(stdout,"Socket creation fail:[%s]\n",strerror(errno));
		exit(EXIT_FAILURE);
	}                
	timeout.tv_sec=3;
	timeout.tv_usec=0;
	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout))<0)
	{
		fprintf(stdout,"\nFAIL TO SET TIMEOUT\n");
	}
		
	serv_addr_size=sizeof(servaddr);
	/*Checking whether the server is alive or not*/
	try=3;
	while(1)
	{	
		memset(&msg,0,sizeof(msg));
		msg.identifier=PING;
		sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr *)&servaddr,serv_addr_size);
		
		recvfrom(sockfd,&msg,sizeof(msg),0,NULL,0);
		
		if(msg.identifier==ALREADYR)	
		{	
			fprintf(stdout,"\n%s\n",msg.msg);
			break;
	
		}
		else{
			if(msg.identifier==REGISTRATION)
			{
				Register_at_server(sockfd,servaddr);
				break;
			}
		}
		try--;
		if(!try)
		{
			fprintf(stdout,"\nServer is not alive\n");
			exit(EXIT_FAILURE);
		}
		
	
	}
	timeout.tv_sec=10000;
	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout))<0)
	{
		fprintf(stdout,"FAIL TO RESET SOCKET\n");
		exit(EXIT_FAILURE);
	}
	Y:
	flag=0;
	while(1)
	{
		fprintf(stdout,"\nUserID of another client:");
        	fgets(C_name,C_Len,stdin);
        	size_t length = strlen(C_name) - 1;
                while (isspace(C_name[length])) {
                        C_name[length] = 0;
                        length--;
                }
		/*check another client UserID should be filled*/
		 if(!strcmp(C_name,""))
                {
                        fprintf(stdout,"\nYou Cannot leave it blank..\n");
                        continue;
                }
                else
                        break;
	}
	detail.sockfd=sockfd;
	detail.ip=argv[1];
	detail.port=argv[2];
	/*thread for sending message*/	
	res=pthread_create(&sendthread,NULL,send_msg,(void*)(&detail));
	if(res)
	{
        	fprintf(stderr,"Error - pthread_create() return code: %d\n",res);
         	exit(EXIT_FAILURE);
	}

	/*thread for recieving message*/	
	res=pthread_create(&recvthread,NULL,recieve_msg,(void*)(&detail));
	if(res)
	{
         	fprintf(stderr,"Error - pthread_create() return code: %d\n",res);
         	exit(EXIT_FAILURE);
	}

	pthread_join(sendthread,NULL);	
	if(flag==1)
	{
		goto Y;
	}
	close(sockfd);			
	return 0;		
}
/*
 * @function    : Register_at_server 
 * @params      : 
 *	@params1: socket for communication
 * 	@params2: structure for registration
 *	@params3: address of server
 * @return      : status of registration
 * @brief       : Function send the detail of client to server for registration
		  and recieve the acknowledgement
 */
void Register_at_server(int sockfd,struct sockaddr_in servaddr)
{
	int ack=0;
	socklen_t serv_addr_size;	
	Registration=(MSG *)malloc(sizeof(MSG));	
	X:
	while(1)
	{
		fprintf(stdout,"\nUSERID:");
		fgets(Registration->C_name,MAX,stdin);
		size_t length = strlen(Registration->C_name) - 1;
		
	
                while (isspace(Registration->C_name[length])) {
                        Registration->C_name[length] = 0;
                        length--;
                }
		/*User ID must be filled*/
		if(!strcmp(Registration->C_name,""))
                {       
                        fprintf(stdout,"\nYou Cannot leave it blank..\n");      
                        continue;
                }
		else
			break;


	}
		
	strcpy(user_name,Registration->C_name);
	strcpy(Registration->msg,"Registration");
	Registration->identifier=REGISTRATION;
	serv_addr_size=sizeof(servaddr);

	sendto(sockfd,Registration,sizeof(MSG),0,(struct sockaddr *)&servaddr,serv_addr_size);
	recvfrom(sockfd,Registration,sizeof(MSG),0,(struct sockaddr *)&servaddr,&serv_addr_size);
	fprintf(stdout,"\n%s:%s\n",Registration->C_name,Registration->msg);
	if(Registration->identifier==DUPLICATE)
	{
		goto X;
	}

}

/*
 * @function    : send_msg
 * @params      : 
 *	@params1: server information
 * @return      : none 
 * @brief       : Function send message to server
 */

void *send_msg(void *arg)
{	
	MSG msg;
	int msglen;
	socklen_t serv_addr_size;
	sock_serv_detail *detail=(sock_serv_detail*)arg;
	struct sockaddr_in servaddr;
	serv_addr_size=sizeof(servaddr);
	
	memset((&servaddr),0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(atoi(detail->port));
	servaddr.sin_addr.s_addr=inet_addr(detail->ip);
	memset((&msg),0,sizeof(msg));
	strcpy(msg.C_name,C_name);
	while(1)
	{
		fgets(msg.msg,MAX,stdin);
		if(!strcasecmp(msg.msg,"LOGOUT\n"))
                {
			msg.identifier=LOGOUT;
			sendto(detail->sockfd,&msg,sizeof(msg),0,(struct sockaddr *)&servaddr,serv_addr_size);
			pthread_exit(0);
                }
		if(!strcasecmp(msg.msg,"EXIT\n"))
		{
			flag=1;
			pthread_exit(0);
		}
		msglen=sizeof(msg);
		sendto(detail->sockfd,&msg,msglen,0,(struct sockaddr *)&servaddr,serv_addr_size);
		
	}
}	

/*
 * @function    : recv_msg
 * @params      : 
 *	@params1: server information
 * @return      : none
 * @brief       : Function recieve message from server
 */
void *recieve_msg(void *arg)
{
	MSG msg;
	struct sockaddr_in clientaddr;
	sock_serv_detail *detail=(sock_serv_detail*)arg;
	memset(&clientaddr,0,sizeof(clientaddr));
	socklen_t client_addr_size=sizeof(clientaddr);

	
	while(1)
	{
		
		recvfrom(detail->sockfd,&msg,sizeof(MSG),0,(struct sockaddr *)&clientaddr,&client_addr_size);
		/*checking whether to exit from this thread*/
		if(flag==1)
		{
			flag=0;
			pthread_exit(0);
		}
		/*checking whether msg package is recieved from server*/
		if(!strcasecmp(detail->ip,inet_ntoa(clientaddr.sin_addr)) )
		{
			fprintf(stdout,"\n%s:%s\n",msg.C_name,msg.msg);
			fflush(stdout);
		}
			
		
	}
}

