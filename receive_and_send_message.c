/*
 * Program which sets up a TCP server.
 * The server can both receive and send messages to the connected client.
 * The program takes one argument and that is the port which the server shall listen to.
 * To close the program write quit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_INPUT 255
#define BACKLOG 50

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
	int sockfd = *((int*)arg),
	    sendBytes = 0,
	    returnValue = 0;
	char sendBuf[MAX_INPUT];
	for (;;)
	{
		memset(sendBuf,0,MAX_INPUT);
		if (fgets(sendBuf,MAX_INPUT,stdin) == NULL)
		{
			fprintf(stderr,"Read interrupted.\n");
		}
		if (strncmp(sendBuf,"quit",4) == 0)
		{
			break;
		}
		if ((sendBytes = send(sockfd,sendBuf,strlen(sendBuf)+1,0)) < 0)
		{
			perror("send");
		}
		else
		{
			printf("Sendt message %s, which is %d bytes on socket %d.\n",sendBuf,sendBytes,sockfd);
		}
	}
	pthread_exit((void*)&returnValue);
}

void *recvThread(void *arg)
{
	printf("Receive thread started with thread id %lu.\n",pthread_self());
	int sockfd = *((int*)arg),
	    recvBytes = 0,
	    returnValue = 0;
	char recvBuf[MAX_INPUT];
	for (;;)
	{
		memset(recvBuf,0,MAX_INPUT);
		if ((recvBytes = recv(sockfd,recvBuf,MAX_INPUT,0)) < 0)
		{
			perror("recv");
		}
		else if (recvBytes == 0)
		{
			printf("Client hung up.\n");
			break;
		}
		else
		{
			printf("Received message %s, which is %d bytes on socket %d.\n",recvBuf,recvBytes,sockfd);
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
	int sockfd,
	    newfd,
	    y = 0,
	    rv;
	struct sockaddr_storage remoteAddr;
	socklen_t addrLen;
	struct addrinfo hints,
			*servInfo,
			*p;
	char *remoteIP;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL,argv[1],&hints,&servInfo)) < 0)
	{
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
		exit(1);
	}
	for (p = servInfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) < 0)
		{
			perror("socket");
			continue;
		}
		if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&y,sizeof(int)) < 0)
		{
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd,p->ai_addr,p->ai_addrlen) < 0)
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
	if (listen(sockfd,BACKLOG) < 0)
	{
		perror("listen");
		exit(1);
	}
	for (;;)
	{
		addrLen = sizeof(remoteAddr);
		if ((newfd = accept(sockfd,(struct sockaddr*)&remoteAddr,&addrLen)) < 0)
		{
			perror("accept");
			continue;
		}
		remoteIP = (char*)malloc(INET6_ADDRSTRLEN);
		if (inet_ntop(remoteAddr.ss_family,get_in_addr((struct sockaddr*)&remoteAddr),remoteIP,INET6_ADDRSTRLEN) == NULL)
		{
			perror("inet_ntop");
			free(remoteIP);
			close(newfd);
			continue;
		}
		remoteIP = (char*)realloc(remoteIP,strlen(remoteIP)+1);
		printf("Established connection from ip %s on port %s.\n",remoteIP,argv[1]);
		free(remoteIP);
		if (pthread_create(&sendTid,NULL,sendThread,(void*)&newfd) != 0)
		{
			fprintf(stderr,"Creation of the send thead failed.\n");
		}
		if (pthread_create(&recvTid,NULL,recvThread,(void*)&newfd) != 0)
		{
			fprintf(stderr,"Creation of the recveive thread failed.\n");
		}
		if (pthread_join(recvTid,&thrRv) != 0)
		{
			fprintf(stderr,"Can't join with the receive thread.\n");
		}
		else
		{
			printf("The receive thread has ended with return code %d.\n",*((int*)thrRv));
			if (pthread_cancel(sendTid) != 0)
			{
				fprintf(stderr,"Unable to cancel the send thread.\n");
			}
			if (pthread_join(sendTid,&thrRv) != 0)
			{
				fprintf(stderr,"Can't join with the send thread.\n");
			}
			else
			{
				printf("The send thread has ended.\n"); // with return code %d.\n",*((int*)thrRv));
			}
		}
		close(newfd);
	}
	close(sockfd);
	exit(0);
}
