/*
 * Program which connects to a server over TCP and can both send to and receive messages from the server.
 * The program takes two arguments:
 * The first argument is the ip address of the server.
 * The second argument is the port of the server.
 * To close the program write quit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAX_INPUT 255

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
		if ((recvBytes = recv(sockfd,recvBuf,MAX_INPUT,0)) > 0)
		{
			printf("Received message %s, which is %d bytes on socket %d.\n",recvBuf,recvBytes,sockfd);
		}
		else if (recvBytes < 0)
		{
			perror("recv");
		}
	}
	pthread_exit((void*)&returnValue);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr,"usage: %s <receiver ip> <reciver port>\n",argv[0]);
		exit(1);
	}
	int sockfd,
	    err;
	struct addrinfo hints,
			*servinfo,
			*p;
	pthread_t sendTid,
		  recvTid;
	void *sendRv,
	     *recvRv;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((err = getaddrinfo(argv[1],argv[2],&hints,&servinfo)) < 0)
	{
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(err));
		exit(1);
	}
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) < 0)
		{
			perror("socket");
			continue;
		}
		if (connect(sockfd,p->ai_addr,p->ai_addrlen) < 0)
		{
			close(sockfd);
			perror("connect");
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	if (p == NULL)
	{
		fprintf(stderr,"connect failed\n");
		exit(1);
	}
	p = NULL;
	if (pthread_create(&sendTid,NULL,sendThread,(void*)&sockfd) != 0)
	{
		fprintf(stderr,"Can't create send thread.\n");
		close(sockfd);
		exit(1);
	}
	if (pthread_create(&recvTid,NULL,recvThread,(void*)&sockfd) != 0)
	{
		fprintf(stderr,"Can't create receive thread.\n");
		close(sockfd);
		exit(1);
	}
	if (pthread_join(sendTid,&sendRv) != 0)
	{
		fprintf(stderr,"Can't join with the send thread.\n");
	}
	else
	{
		printf("Send thread ended with return value %d.\n",*((int*)sendRv));
		if (pthread_cancel(recvTid) != 0)
		{
			fprintf(stderr,"Could not cancel the receive thread.\n");
		}
		if (pthread_join(recvTid,&recvRv) != 0)
		{
			fprintf(stderr,"Can't join with the receive thread.\n");
		}
		else
		{
			printf("Receive thread ended.\n"); // with return value %d.\n",*((int*)recvRv));
		}
	}
	printf("Ends the program.\n");
	close(sockfd);
	exit(0);
}
