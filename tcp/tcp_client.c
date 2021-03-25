#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
 
#define PORT  5555
#define BUF_SIZE 1024
#define LISTEN_LENGTH 20
 
int sockfd;
int fp;
int recvBytes,sendBytes,readBytes; 
unsigned char buffer[BUF_SIZE];
struct hostent *host;
struct sockaddr_in serv_addr;

char cmdInfo[BUF_SIZE];
char cmdInfo1[BUF_SIZE];

void tcp_usage_client(char ** argv)
{
	printf("1. example: %s 192.168.1.2 upload /home/fsm/x86linuxcompiletest2.zip\n",argv[0]);
	printf("1. example: %s 192.168.1.2 compile x86linuxcompiletest2.zip \n",argv[0]);
	printf("1. example: %s 192.168.1.2 deploy x86linuxcompiletest2.zip \n",argv[0]);
	return;
}

int tcp_init(char ** argv)
{
	int iret = 0;

	//parser hostname
	host = gethostbyname(argv[1]);
	if(host == NULL){
		printf("host name error!\n");
		return 1;
	}
	//create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		printf("socket create error!\n");
		return 1;
	}
	//request connect to server
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr = *((struct in_addr*)host->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
	iret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
	if(iret == -1){
		printf("connect error!\n");
		return 1;
	}
	printf("connect success!\n");
	return 0;
}

int tcp_upload_file(char ** argv)
{
	//open file for send
	fp = open(argv[3], O_RDONLY, 0777);
	if(-1 == fp){
		printf("%s file open failed !!!\n", argv[2]);
		exit(1);
	}
	else{
		printf("open %s file success!!\n", argv[2]);
	}
	
	//communicate
	//step 1. send upload command
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, argv[2]);
	send(sockfd, buffer, strlen(buffer)+1, 0);
	printf("send cmdinfo finished!\n");

	memset(buffer, 0, sizeof(buffer));
	recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
	printf("received %d bytes, server received cmd = %s\n",recvBytes, buffer);
	if(strcmp(buffer, "upload") != 0){
		printf("upload command send failed!\n");
		return 1;
	}
	
	//step 2. send file name
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, argv[3]);
	send(sockfd, buffer, strlen(buffer)+1, 0);
	printf("send filename finished!\n");

	memset(buffer, 0, sizeof(buffer));
	recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
	printf("received %d bytes, received data = %s\n",recvBytes,buffer);
 
   	//step 3. send file data
   	memset(buffer, 0, sizeof(buffer));
        while ((readBytes = read(fp, buffer, BUF_SIZE)) >0 )
	{
      		sendBytes = send(sockfd, buffer, readBytes, 0);
     	 	if ( sendBytes < 0 ) {
           		printf("！！！Send file failed！！！\n");
           		exit(1);
       		}
      	  	memset(buffer,0,sizeof(buffer));
   	}

	printf("Finsh file data send\n");
	close(fp);

	return 0;
}

int tcp_compile(char ** argv)
{

	//communicate
	//step 1. send compile command
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, argv[2]);
	send(sockfd, buffer, strlen(buffer)+1, 0);
	printf("send compile cmd finished!\n");

	memset(buffer, 0, sizeof(buffer));
	recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
	printf("received %d bytes, server received cmd = %s\n",recvBytes, buffer);
	if(strcmp(buffer, "compile") != 0){
		printf("compile command send failed!\n");
		return 1;
	}
	
	//step 2. send compile project name
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, argv[3]);
	send(sockfd, buffer, strlen(buffer)+1, 0);
	printf("send compile project name :%s finished!\n", buffer);

	memset(buffer, 0, sizeof(buffer));
	recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
	printf("received %d bytes, received project name = %s\n",recvBytes, buffer);
	return 0;
}

int tcp_deploy(char ** argv)
{

	//communicate
	//step 1. send compile command
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, argv[2]);
	send(sockfd, buffer, strlen(buffer)+1, 0);
	printf("send deploy cmd finished!\n");

	memset(buffer, 0, sizeof(buffer));
	recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
	printf("received %d bytes, server received cmd = %s\n",recvBytes, buffer);
	if(strcmp(buffer, "deploy") != 0){
		printf("deploy command send failed!\n");
		return 1;
	}
	
	//step 2. send compile project name
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, argv[3]);
	send(sockfd, buffer, strlen(buffer)+1, 0);
	printf("send deploy project name :%s finished!\n", buffer);

	memset(buffer, 0, sizeof(buffer));
	recvBytes = recv(sockfd, buffer, sizeof(buffer), 0);
	printf("received %d bytes, received project name = %s\n",recvBytes, buffer);
	return 0;
}


int main(int argc, char* argv[])
{
	int iret = 0;

	if(argc != 4){
		tcp_usage_client(argv);
	}

	iret = tcp_init(argv);
	if(iret != 0){
		printf("tcp init failed!\n");
		return 1;
	}

	if(strcmp(argv[2], "upload") == 0){
		iret = tcp_upload_file(argv);
		if(iret != 0){
			printf("tcp upload file failed!\n");
			return 1;
		}
	}else if(strcmp(argv[2], "compile") == 0){
		iret = tcp_compile(argv);
		if(iret != 0){
			printf("tcp compile project failed!\n");
			return 1;
		}
	}else if(strcmp(argv[2], "deploy") == 0){
		iret = tcp_deploy(argv);
		if(iret != 0){
			printf("tcp deploy project failed!\n");
			return 1;
		}
	}

	close(sockfd);
	printf("client quit!\n");

	return 0;
}
