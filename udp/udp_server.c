#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define PORT 8888
#define Name 100
 
int sockfd,n;
struct sockaddr_in server,client;
unsigned char cmdInfo[BUFFER_SIZE];
unsigned char cmdInfo1[BUFFER_SIZE];

char filename[100];
char filepath[100];
char filePath[100];
char file_no_dot[100];
unsigned char filePath1[Name],filePath2[Name];

char buffer[BUFFER_SIZE];//file buffer
int fileTrans;

int lenfilepath;
FILE *fp;
int writelength;

int addrlen;

void * udp_net_process(void * arg);
void udp_receive_cmd_process();
int udp_receive_file_process();
int udp_server_init();
void udp_compile_process();
void udp_deploy_process();

int udp_server_init()
{
    	addrlen=sizeof(struct sockaddr);
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
        	printf("socket create error!\n");
		return -1;
    	}	
    	else{
        	printf("socket create success!\n");
    	}

    	memset(&server,0,sizeof(server));
    	server.sin_family= AF_INET;
    	server.sin_addr.s_addr = htonl(INADDR_ANY);
    	server.sin_port = htons(PORT);
 
    	if((bind(sockfd, (struct sockaddr*)&server, sizeof(server))) == -1){
        	printf("bind error!\n");
		return -1;
    	}
    	else{
        	printf("bind success!\n");
    	}

	return 0;
}

void * udp_net_process(void * arg)
{
        int iret = 0;

        while(1){
                udp_receive_cmd_process();
                if(strcmp(cmdInfo, "upload") == 0){
                        udp_receive_file_process();
                }else if(strcmp(cmdInfo, "compile") == 0){
                        udp_compile_process();
                }else if(strcmp(cmdInfo, "deploy") == 0){
                        udp_deploy_process();
                }else{
                        printf("recevie cmd:%s is not correct!\n", cmdInfo);
                }
        }
        return NULL;
}

void udp_receive_cmd_process()
{
        int recvBytes = 0;
        bzero(cmdInfo, BUFFER_SIZE);
        bzero(cmdInfo1, BUFFER_SIZE);

        recvBytes = recvfrom(sockfd, cmdInfo, sizeof(cmdInfo) , 0, (struct sockaddr *)&client, &addrlen);
        strcpy(cmdInfo1,  cmdInfo);
        printf("received %d bytes, cmd=%s\n", recvBytes, cmdInfo1);

	/*
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, cmdInfo1);
        send(sockfd, buffer, strlen(buffer)+1, 0);
	*/

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

void udp_compile_process()
{
        int recvBytes = 0;
        char compileCmd[100];
        char compileResult[100];
	FILE *pfd = NULL;
	FILE *pfd1 = NULL;

        bzero(compileCmd, 100);

        //step 1. transmit file name
        bzero(buffer, BUFFER_SIZE);
        recvBytes = recvfrom(sockfd, cmdInfo, sizeof(cmdInfo) , 0, (struct sockaddr *)&client, &addrlen);
        //recvBytes = recv(sockfd, buffer, BUF_SIZE, 0);
        strcpy(filePath, cmdInfo);
        printf("received %d bytes, compile project name=%s\n", recvBytes, filePath);

        //memset(buffer, 0, sizeof(buffer));
        //strcpy(buffer, "compile project received success!\n");
        //send(sockfd, buffer, strlen(buffer)+1, 0);

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


	//pclose(pfd);
        //system(compileCmd);
	
        return;
}


void udp_deploy_process()
{
        int recvBytes = 0;

        //step 1. transmit file name
        bzero(buffer, BUFFER_SIZE);
        recvBytes = recvfrom(sockfd, cmdInfo, sizeof(cmdInfo) , 0, (struct sockaddr *)&client, &addrlen);
        //recvBytes = recv(sockfd, buffer, BUF_SIZE, 0);
        strcpy(filePath, cmdInfo);
        printf("received %d bytes, deploy project name=%s\n", recvBytes, filePath);
/*
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "deploy project received success!\n");
        send(sockfd, buffer, strlen(buffer)+1, 0);
*/
        printf("deploy prioject:%s.......\n",filePath);
        return;
}

int udp_receive_file_process()
{
	memset(filename,'\0',sizeof(filename));
	memset(filepath,'\0',sizeof(filepath));
        lenfilepath = recvfrom(sockfd, filepath, 100, 0, (struct sockaddr *)&client, &addrlen);

	//step 1. receive file name.
        printf("receive filename :%s\n", filepath);
        if(lenfilepath < 0){
		printf("recv error!\n");
		return -1;
	}else{
                int i=0,k=0;  
                for(i=strlen(filepath);i>=0;i--)  
                {  
                    if(filepath[i]!='/')      
                    {  
                        k++;  
                    }  
                    else{   
                        break;
		    }	
                }  
                strcpy(filename, filepath + (strlen(filepath) - k) + 1);   
	}

	//step 2. receive file data.
       printf("filename :%s\n", filename);
       fp = fopen(filename,"w");
       if(fp != NULL)
       {
		//int times = 1;
                while(fileTrans =recvfrom(sockfd,buffer,BUFFER_SIZE,0,(struct sockaddr *)&client,&addrlen))
                {
			//printf("times = %d   ",times);
			//times++;
                    if(fileTrans<0)
                    {
                        printf("recv2 error!\n");
                        break;
                    }
 
                    writelength = fwrite(buffer,sizeof(char),fileTrans,fp);
 
                    if(fileTrans < BUFFER_SIZE)
                    {
                        printf("finish writing!\n");
                        break;
                    }else{
			//printf("write succ! %d fileTrans=%d\n",writelength,fileTrans);
			//printf("write successful!\n");
			//break;
		    }
			//printf("continue\n");
                    	bzero(buffer,BUFFER_SIZE); 
                }
                printf("recv finished!\n");
                fclose(fp);
	}else{
                printf("filename is null!\n");
 	}
}

int main()
{
	char cmd[10];
	pthread_t thread;

    	bzero(buffer,BUFFER_SIZE); 

	udp_server_init();
 
	pthread_create(&thread, NULL, udp_net_process, (void*)(void *)(0));

        scanf("%s", cmd);
        if(strcmp(cmd, "q") == 0){
                close(sockfd);
        }

//      for(;;);
        return 0;
}
