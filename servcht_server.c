/**
 * @file       : server.c 
 * @brief      : Server performs the chat between the client
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#define PORT 43453
#define REGISTRATION 1
#define LOGOUT 2
#define DUPLICATE 3


/*
 * @structure   : Clients 
 * @brief       : struct which defines client name and IP  address 
 * @members     :
 *      @mem1   : holds client name
 *      @mem2   : holds message
 *	@mem3	: check whether request is for registration,logout,duplicate
 */


typedef struct client 
{
char name[15];
char msg[100];
int id;
}Clients;


/*
 * @structure   : Client_info 
 * @brief       : struct which stores the clients name and IP  address 
 * @members     :
 *      @mem1   : holds client name
 *      @mem2   : hold IP address
 *	@mem3	: holds the address of next details of client
 */


typedef struct client_info
{
char name[15];
char ip[15];
struct client_info *next;
}CI;


/*
 * @function    : storeinfo 
 * @params      :
 *      @param1 : structure variable that point to the starting of client detail 
 *	@param2	: Name of client
 *	@param3	: IP of client
 *                  
 * @return      : function status
 * @brief       : this function stores the detail of clients
 *                on the server
 */


void storeinfo(CI **n,char a[],char b[])
{
        CI *temp=*n;
        CI *extra=(CI *)malloc(sizeof(CI));
	strcpy(extra->name,a);
	strcpy(extra->ip,b);
	extra->next=NULL;
        if(*n==NULL)
        {
                *n=extra;
                return;
        }
        while(temp->next!=NULL)
                temp=temp->next;
        temp->next=extra;
        return;
}


/*
 * @function    : print
 * @params      :
 *      @param1 : structure variable for accessing the information of clients
 * @return      : function status
 * @brief       : this function print the info of available clients 
 */


void print(CI* n)
{
		int i=0;
		while(n!=NULL)
		{	
			
			i=1;
			fprintf(stdout,"\nRemaining Client\n");
			fprintf(stdout,"%s::%s\n\n",n->name,n->ip);
			fflush(stdout);
			n=n->next;
		}
	if(i==0)
	fprintf(stdout,"\n\nNo Client registered\n");
	fflush(stdout);
        return;
}


/*
 * @function    : lout
 * @params      :
 *      @param1 : structure for getting the info of available clients
 *      @param2 : Logged client IP
 * @return      : function status
 * @brief       : this function deleted the info of logged client
 */


char* lout(CI **n,char a[15])
{

        CI* temp = *n, *prev;
	static char name[15];
        if (temp != NULL && (!strcmp(temp->ip,a))) 
        {
		strcpy(name,temp->name);
                *n = temp->next;
                free(temp);
                return name;
        }
        while (temp != NULL && (strcmp(temp->ip,a)))
        {
                prev = temp;
                temp = temp->next;
        }
        if (temp == NULL) return "hacker"; 
        strcpy(name,temp->name);	
        prev->next = temp->next;
        free(temp);
	return name;
}


/*
 * @function    : main 
 * @params      :
 * @return      : 
 * @brief       : Server main 
 */


int main()
{
	int sockfd, newsockfd, portno,i,check=0;
	char client_ath='N',client_av='N',*lname,cregister='N',client_resend='N';
	char extraip[15],extraname[15];
	char namer[50],names[50];
	struct sockaddr_in serv_addr, send_addr,recv_addr;
	CI *temp, *clientinfo=NULL;
	Clients *c_name;
	socklen_t sendsize,recvsize;
	sendsize = sizeof(send_addr);
	recvsize=sizeof(recv_addr);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
		fprintf(stdout,"Error opening socket\n");
	else
	fprintf(stdout,"Socket Created\n");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);
	if (bind(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))!=0) 
		fprintf(stdout,"Socket binding failed\n");
	else
		fprintf(stdout,"Socket binded Successfully\n");
	recv_addr.sin_family = AF_INET;
        recv_addr.sin_port = htons(PORT);
	while(1)
	{	
		temp=clientinfo;
		c_name=(Clients *)malloc(sizeof(Clients));
		recvfrom(sockfd,c_name,sizeof(Clients),0,(struct sockaddr *)&send_addr,&sendsize);



		/*ckeck whether client want to logout or not */

		if(c_name->id==LOGOUT)
		{		
			strcpy(extraip,inet_ntoa(send_addr.sin_addr));
			lname=lout(&clientinfo,extraip);
			if(strcasecmp(lname,"hacker")!=0)
			{
				fprintf(stdout,"\n\n%s is logout",lname);
				fflush(stdout);
				print(clientinfo);
				temp=clientinfo;
				while(temp!=NULL)
					{
						if(strcmp(c_name->name,temp->name)==0)
							{
								recv_addr.sin_addr.s_addr = inet_addr(temp->ip);
								check=1;
								break;
								
							}
						temp=temp->next;
					}
				if(check==1)
				{
					strcpy(c_name->name,"Server");
					strcat(lname," is logged out");
					strcpy(c_name->msg,lname);
					sendto(sockfd,c_name,sizeof(Clients),0,(struct sockaddr *)&recv_addr,recvsize);
				}
			}
		}



		/*check whether client want to register or not*/		

		else if(c_name->id==REGISTRATION)
		{
			strcpy(extraip,inet_ntoa(send_addr.sin_addr));   
			strcpy(extraname,c_name->name);

			temp=clientinfo;

			/*check whether user id is unique*/
			while(temp!=NULL)
                        {
                                if((strcasecmp(extraname,temp->name)==0))
                                {
                                        c_name->id=DUPLICATE;
                                        strcpy(c_name->msg,"This name is already taken,try it with different username ");
                                        strcpy(c_name->name,"Server");
                                        break;
                                }
                                temp=temp->next;
                        }

			if(c_name->id==DUPLICATE)
				sendto(sockfd,c_name,sizeof(Clients),0,(struct sockaddr *)&send_addr,sendsize);

			else
			{
				/*check whether registering client is already registered or not*/
				temp=clientinfo;
				while(temp!=NULL)
				{
					if((strcasecmp(extraip,temp->ip)==0))
					{
						cregister='Y';
						strcpy(c_name->name,temp->name);
						strcpy(c_name->msg,"You are already registered as ");
						strcat(c_name->msg,c_name->name);
						strcpy(c_name->name,"Server");
						break;
					}
					temp=temp->next;
				}
				if(cregister=='Y')
				{
					sendto(sockfd,c_name,sizeof(Clients),0,(struct sockaddr *)&send_addr,sendsize);
					cregister='N';
				}
				/*Registering client*/
				else
				{	
					strcpy(extraip,inet_ntoa(send_addr.sin_addr));
					strcpy(extraname,c_name->name);
					storeinfo(&clientinfo,extraname,extraip);
					fprintf(stdout,"\nRegistered Clients\n");
					fprintf(stdout,"\n\n%s::%s::%s",c_name->name,inet_ntoa(send_addr.sin_addr),c_name->msg);
					fflush(stdout);
					strcpy(c_name->name,"Server");
					strcpy(c_name->msg,"You are successfully registered");
					sendto(sockfd,c_name,sizeof(Clients),0,(struct sockaddr *)&send_addr,sendsize);	
				}
			}
		}


		/*For client communication*/
		else
		{
			temp=clientinfo;
			/*check the authenticity of sender*/
			while(temp!=NULL)
			{
				if(strcmp(inet_ntoa(send_addr.sin_addr),temp->ip)==0)
				{
					client_ath='Y';
					strcpy(names,temp->name);
					break;
				}
				temp=temp->next;
			}
			if(client_ath=='Y')
			{
				temp=clientinfo;
				/*Check the availability of recipient client*/
				while(temp!=NULL)
				{
					if((strcmp(c_name->name,temp->name)==0)&&(strcmp(c_name->name,names)!=0))
					{	
						client_av='Y';
               			                recv_addr.sin_addr.s_addr = inet_addr(temp->ip);
						strcpy(c_name->name,names);
						break;
                       			}
					temp=temp->next;
				}
				/*check whether sending client is not sending yourself*/
				if(strcasecmp(c_name->name,names)==0)
					client_resend='Y';
				if(client_av=='Y')
				{
					sendto(sockfd,c_name,sizeof(Clients),0,(struct sockaddr *)&recv_addr,recvsize);
					client_av='N';
				}
				else
				{
					if(client_resend=='Y')
					{
						strcpy(c_name->name,"Server");
						strcpy(c_name->msg,"You cannot send yourself");
						client_resend='N';
					}
					else
					{
						strcpy(c_name->msg,c_name->name);
						strcpy(c_name->name,"Server");
						strcat(c_name->msg," is not available");
					}
					sendto(sockfd,c_name,sizeof(Clients),0,(struct sockaddr *)&send_addr,sendsize);
				}
				client_ath='N';		
			}
			/*If client is not registered*/
			else
			{ 
				strcpy(c_name->name,"Server");
				strcpy(c_name->msg,"You are not registered");
				c_name->id=REGISTRATION;
				sendto(sockfd,c_name,sizeof(Clients),0,(struct sockaddr *)&send_addr,sendsize);
			}
		}
	}

	return 0; 

}
