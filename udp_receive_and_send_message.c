/*
 * Program which sets up a UDP server.
 * The server can both receive and send messages to the connected client.
 * The program takes one argument and that is the port which the server shall listen to.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_INPUT 255

typedef struct
{
        int sockfd;
        struct sockaddr_storage clientAddr;
        socklen_t addrLen;
} ThrArg;

void *get_in_addr(struct sockaddr *sa)
{
        if (sa->sa_family == AF_INET)
        {
                return &(((struct sockaddr_in*)sa)->sin_addr);
        }
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *sendThread(void *arg)
{
	printf("Send thread started with thread id %lu.\n",pthread_self());
        // I do not need to lock the info structure becuse sendThread only reads from it and recvThread only writes to it.
        ThrArg *info = (ThrArg*)arg;
	int sendBytes = 0,
	    returnValue = 0;
	char sendBuf[MAX_INPUT];
        for (;;)
        {
                memset(sendBuf,0,MAX_INPUT);
                if (fgets(sendBuf,MAX_INPUT,stdin) == NULL)
                {
                        fprintf(stderr,"Read interrupted.\n");
                }
                // Can't have an exit command here because the program only listen here if a client is connected.
                // I can lock ThrArg here.
                if ((sendBytes = sendto(info->sockfd,sendBuf,strlen(sendBuf)+1,0,(const struct sockaddr*)&info->clientAddr,info->addrLen)) < 0)
                {
                        perror("sendto");
                }
                else
                {
                        printf("Sendt message %s, which is %d bytes on socket %d.\n",sendBuf,sendBytes,info->sockfd);
                }
        }
        pthread_exit((void*)&returnValue);
}

void *recvThread(void *arg)
{
	printf("Receive thread started with thread id %lu.\n",pthread_self());
        ThrArg *info = (ThrArg*)arg;
	int recvBytes = 0,
	    returnValue = 0;
	char recvBuf[MAX_INPUT];
	for (;;)
	{
		memset(recvBuf,0,MAX_INPUT);
                // I can NOT lock ThrArg here because recvBytes waits
                if ((recvBytes = recvfrom(info.sockfd,recvBuf,MAX_INPUT,0,(struct sockaddr*)&info->clientAddr,&info->addrLen)) < 0)
                {
                        perror("recvfrom");
                }
                else
                {
			printf("Received message %s, which is %d bytes on socket %d.\n",recvBuf,recvBytes,info->sockfd);
                }
        }
        pthread_exit((void*)&returnValue);
}

int main(int argc,char *argv[])
{
        if (argc != 2)
        {
                fprintf(stderr,"usage %s <port>\n",argv[0]);
                exit(1);
        }
	pthread_t sendTid,
		  recvTid;
	void *thrRv;
        ThrArg info;
        int rv;
        struct sockaddr_storage remoteAddr;
        struct addrinfo hints,
                        *servInfo,
                        *p;
        char *remoteIP;
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;
        if ((rv = getaddrinfo(NULL,argv[1],&hints,&servInfo)) < 0)
        {
                fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
                exit(1);
        }
        for (p = servInfo; p != NULL; p = p->ai_next)
        {
                if ((info.sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) < 0)
                {
                        perror("socket");
                        continue;
                }
                if (bind(info.sockfd,p->ai_addr,p->ai_addrlen) < 0)
                {
                        perror("bind");
                        close(sockfd);
                        continue;
                }
                break;
        }
        freeaddrinfo(servInfo);
        if (p == NULL)
        {
                fprintf(stderr,"bind failed.\n");
                exit(1);
        }
        p = NULL;
        for (;;)
        {
