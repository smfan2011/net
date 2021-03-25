#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
 
#define BUFFER_SIZE 1024
#define PORT 8888 

int sockfd;
struct sockaddr_in server;
char filepath[100];//file to translate
int addrlen=sizeof(struct sockaddr);
 
FILE *fp;
int lenpath; //filepath length
//char *buffer;//file buffer
int fileTrans;

char buffer[BUFFER_SIZE];


void udp_usage_client(char ** argv)
{
        printf("1. example: %s 192.168.1.2 upload /home/fsm/x86linuxcompiletest2.zip\n",argv[0]);
        printf("1. example: %s 192.168.1.2 compile x86linuxcompiletest2.zip \n",argv[0]);
        printf("1. example: %s 192.168.1.2 deploy x86linuxcompiletest2.zip \n",argv[0]);
        return;
}


int udp_init(char ** argv)
{
	int iret = 0;
        //parser hostname
	//host = gethostbyname(argv[1]);

    	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if( sockfd < 0 ){
        	printf("socket build error!\n");
		return 1;
    	}
    	memset(&server,0,sizeof(server));
    	server.sin_family= AF_INET;
    	server.sin_port = htons(PORT);

	//serv_addr.sin_addr = *((struct in_addr*)host->h_addr);
        //bzero(&(serv_addr.sin_zero), 8);

    	iret = inet_pton(AF_INET, argv[1], &server.sin_addr);
    	if(iret < 0){
        	printf("inet_pton error!\n");
		return 1;
    	}

	return 0;
}

int udp_send_file_process(char ** argv)
{
    bzero(buffer,BUFFER_SIZE); 
 
 
    fp = fopen(argv[3], "r");//opne file
    if(fp == NULL){
        printf("err:file not found!\n");
        return 1;
    }

   //step 1. send cmd
   
    printf("send command : %s\n",argv[2]);
    lenpath = sendto(sockfd, argv[2], strlen(argv[2]), 0, (struct sockaddr *)&server, addrlen);// put file path to sever 
    if(lenpath < 0){
        printf("command send error!\n");
    }
    else{
        printf("command send success!\n");
    }

    //sleep(1);

   //step 2. send file name
    printf("send file name : %s\n",argv[3]);
    lenpath = sendto(sockfd, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&server, addrlen);// put file path to sever 
    if(lenpath < 0){
        printf("file name send error!\n");
    }
    else{
        printf("file name  send success!\n");
    }

    //sleep(1);
    
    // step 3. send file date
    printf("begin send data...\n");
    while((fileTrans = fread(buffer, sizeof(char), BUFFER_SIZE, fp))>0)
    {
	//sleep(1);  //take care here.
        if(sendto(sockfd, buffer, fileTrans, 0, (struct sockaddr *)&server, addrlen)<0){
            printf("send failed!\n");
            break;      
        }
	if(fileTrans < BUFFER_SIZE){
	       	break;
	}

        bzero(buffer,BUFFER_SIZE); 
    }
    fclose(fp);
}

int udp_compile(char ** argv)
{

        //communicate
        //step 1. send compile command
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, argv[2]);
    	lenpath = sendto(sockfd, argv[2], strlen(argv[2]), 0, (struct sockaddr *)&server, addrlen);// put file path to sever 
        printf("send compile cmd finished!\n");

	/*
        memset(buffer, 0, sizeof(buffer));
        recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
        printf("received %d bytes, server received cmd = %s\n",recvBytes, buffer);
        if(strcmp(buffer, "compile") != 0){
                printf("compile command send failed!\n");
                return 1;
        }*/

        //step 2. send compile project name
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, argv[3]);
    	lenpath = sendto(sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&server, addrlen);// put file path to sever 
        //send(sockfd, buffer, strlen(buffer)+1, 0);
        printf("send compile project name :%s finished!\n", buffer);
/*
        memset(buffer, 0, sizeof(buffer));
        recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
        printf("received %d bytes, received project name = %s\n",recvBytes, buffer);
*/
        return 0;
}

int udp_deploy(char ** argv)
{

        //communicate
        //step 1. send compile command
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, argv[2]);
    	lenpath = sendto(sockfd, argv[2], strlen(argv[2]), 0, (struct sockaddr *)&server, addrlen);// put file path to sever 
        //send(sockfd, buffer, strlen(buffer)+1, 0);
        printf("send deploy cmd finished!\n");


	/*
        memset(buffer, 0, sizeof(buffer));
        recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
        printf("received %d bytes, server received cmd = %s\n",recvBytes, buffer);
        if(strcmp(buffer, "deploy") != 0){
                printf("deploy command send failed!\n");
                return 1;
        }
	*/

        //step 2. send compile project name
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, argv[3]);
        //send(sockfd, buffer, strlen(buffer)+1, 0);
    	lenpath = sendto(sockfd, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&server, addrlen);// put file path to sever 
        printf("send deploy project name :%s finished!\n", buffer);

	/*
        memset(buffer, 0, sizeof(buffer));
        recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
        printf("received %d bytes, received project name = %s\n",recvBytes, buffer);
	*/
        return 0;
}


int main(int argc, char *argv[])
{
	int iret = 0;
	if(argc != 4){
        	udp_usage_client(argv);
        }

	udp_init(argv);
//	udp_send_file_process(argv);

	if(strcmp(argv[2], "upload") == 0){
                iret = udp_send_file_process(argv);
                if(iret != 0){
                        printf("tcp upload file failed!\n");
                        return 1;
                }
        }else if(strcmp(argv[2], "compile") == 0){
                iret = udp_compile(argv);
                if(iret != 0){
                        printf("tcp compile project failed!\n");
                        return 1;
                }
        }else if(strcmp(argv[2], "deploy") == 0){
                iret = udp_deploy(argv);
                if(iret != 0){
                        printf("tcp deploy project failed!\n");
                        return 1;
                }
        }

    	close(sockfd);
    	return 0;
}
