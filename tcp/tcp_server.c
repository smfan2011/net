#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#define PORT 		5555
#define BUF_SIZE 	1024
#define Name  		100
#define LISTEN_LENGTH 20
 

struct sockaddr_in server_sockaddr,client_sockaddr;
int size, writeBytes, fp;
int sockfd,listenfd;

unsigned char filePath[Name];
unsigned char filePath1[Name],filePath2[Name];
unsigned char buffer[BUF_SIZE];
char file_no_dot[100];
unsigned char cmdInfo[BUF_SIZE];
unsigned char cmdInfo1[BUF_SIZE];

int i=0;

void tcp_receive_cmd_process();
void tcp_receive_file_process();
void tcp_compile_process();
void tcp_deploy_process();

int tcp_init()
{
	int iret = 0;

	//create socket fd for listening
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == listenfd){
		printf("socket create error!\n");
		return 1;
	}
	printf("socket create success! listenfd=%d\n",listenfd);
	//bind ip addr and port number
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(PORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero), 8);
	
	iret = bind(listenfd, (struct sockaddr*)&server_sockaddr, sizeof(struct sockaddr));
	if(-1 == iret ){
		printf("bind error!\n");
		return 1;
	}

	return 0;
}

void * tcp_net_process(void * arg)
{
	int iret = 0;

	while(1){
		//listening
		iret = listen(listenfd, LISTEN_LENGTH);
		if(-1 == iret){
			printf("listen error!\n");
			exit(1);
		}

		//accept
		size = sizeof(client_sockaddr);
		sockfd = accept(listenfd, (struct sockaddr*)&client_sockaddr, &size);
		if(-1 == sockfd){
			printf("accept error!\n");
			exit(1);
		}

		tcp_receive_cmd_process();
		if(strcmp(cmdInfo, "upload") == 0){
			tcp_receive_file_process();
		}else if(strcmp(cmdInfo, "compile") == 0){
			tcp_compile_process();
		}else if(strcmp(cmdInfo, "deploy") == 0){
			tcp_deploy_process();
		}else{
			printf("recevie cmd:%s is not correct!\n", cmdInfo);
		}
		//receive_file_process();
	}
	return NULL;
}

void tcp_receive_cmd_process()
{	
	int recvBytes = 0; 
    	bzero(cmdInfo, BUF_SIZE);
    	bzero(cmdInfo1, BUF_SIZE);

	recvBytes = recv(sockfd, cmdInfo, BUF_SIZE, 0);
	strcpy(cmdInfo1,  cmdInfo);
	printf("received %d bytes, cmd=%s\n", recvBytes, cmdInfo1);

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, cmdInfo1);
	send(sockfd, buffer, strlen(buffer)+1, 0);

	return;
}

void tcp_deploy_process()
{
	int recvBytes = 0; 

	//step 1. transmit file name
    	bzero(buffer, BUF_SIZE);
	recvBytes = recv(sockfd, buffer, BUF_SIZE, 0);
	strcpy(filePath, buffer);
	printf("received %d bytes, deploy project name=%s\n", recvBytes, filePath);

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "deploy project received success!\n");
	send(sockfd, buffer, strlen(buffer)+1, 0);

	printf("deploy prioject:%s.......\n",filePath);
	return;
}

void getStringBeforeDot(char* string) 
{
	char *p_start = string;
    	int num = 0;

    	while(*p_start) {
        	if (*p_start == '.'){
            		break;
		}
        	p_start++;
        	num ++;
    	}

    	bzero(file_no_dot, 100);
    	strncpy(file_no_dot, string, num);
    	printf("newString = %s\n", file_no_dot);
	return;
}

void tcp_compile_process()
{
	int recvBytes = 0;
	char compileCmd[100];
	char compileResult[100];
	FILE * pfd = NULL;
	FILE * pfd1 = NULL;

	bzero(compileCmd, 100);


	//step 1. transmit file name
    	bzero(buffer, BUF_SIZE);
    	bzero(filePath, BUF_SIZE);
	recvBytes = recv(sockfd, buffer, BUF_SIZE, 0);
	strcpy(filePath, buffer);
	printf("received %d bytes, compile project name=%s\n", recvBytes, filePath);

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "compile project received success!\n");
	send(sockfd, buffer, strlen(buffer)+1, 0);

	printf("ccompile prioject:%s.......\n",filePath);
	getStringBeforeDot(filePath);
	printf("file no dot:%s\n", file_no_dot);

        sprintf(compileCmd, "sh compile.sh %s", filePath);
	printf("compileCmd:%s\n",compileCmd);
	pfd = popen(compileCmd, "r");
	if(pfd == NULL){
		printf("popen %s failed!\n", compileCmd);
		return;
	}

        bzero(compileCmd, 100);
        sprintf(compileCmd, "cat %s.log | awk \'END {print}\'", file_no_dot);
	pfd1 = popen(compileCmd, "r");
	if(pfd1 == NULL){
		printf("popen %s failed!\n", compileCmd);
		return;
	}

	while(fgets(compileResult, 100, pfd1) != NULL){
		printf("build result:%s\n", compileResult);
		if(strncmp(compileResult, "failed", 6) == 0){
			printf("build failed!\n");
			return;
		}else if(strncmp(compileResult, "success", 7) == 0){
			printf("build success!\n");
			return;
		}else{
			printf("warning:something wrong.\n");
		}
	}


	//sprintf(compileCmd, "sh compile.sh %s", filePath);
	//system(compileCmd);
	return;
}

void tcp_receive_file_process()
{
	int recvBytes = 0; 

	//step 1. transmit file name
    	bzero(buffer, BUF_SIZE);
	recvBytes = recv(sockfd, buffer, BUF_SIZE, 0);
	strcpy(filePath, buffer);
	printf("received %d bytes,data=%s\n", recvBytes, filePath);

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "filename accept success!\n");
	send(sockfd, buffer, strlen(buffer)+1, 0);
	
	//step 2.get file name, remove path info
	if(NULL != strchr(filePath,'/'))
	{
		strcpy(filePath1, strrchr(filePath, '/'));
		for(i = 1; filePath1[i] != '\0'; i++)
		{
			filePath2[i-1] = filePath1[i];
			filePath2[i] = '\0';
		}
	}
	else{
		strcpy(filePath2, filePath);
	}
	
	//step 3. create file on server 
	fp = open(filePath2, O_CREAT|O_RDWR, 0777);       
	if(-1 == fp){		
		printf("You %s create error!\n", filePath2);
		exit(1);
	}

	printf("You %s file create success!,start accept data!!!\n", filePath2);
	
	//step 4. transmit file
	bzero(buffer, BUF_SIZE);
    	while(recvBytes= recv(sockfd, buffer, BUF_SIZE, 0))
	{	
           	if(recvBytes < 0) {
               		printf("！！！Recieve Data From Server Failed！！！\n");
               		break;
           	}	
         
           	writeBytes = write(fp, buffer, recvBytes);
          	if(writeBytes < recvBytes){
               		printf("！！！！Write file failed！! ! ! ! !\n");
               		break;
          	}

           	bzero(buffer, BUF_SIZE);
        }

	close(fp);
    	printf("Finish Recieve\n");
}

void main()
{
	pthread_t thread;
	char cmd[10];

	tcp_init();
	pthread_create(&thread, NULL, tcp_net_process, (void*)(void *)(0));

	scanf("%s", cmd);
	if(strcmp(cmd, "q") == 0){
		close(sockfd);
		close(listenfd);
	}

//	for(;;);	
	return;
}
