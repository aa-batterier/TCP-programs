/*
 * This program sends messages to a server over TCP.
 * The program takes two arguments.
 * The first argument is the ip address of the server.
 * The second argument is the port of the server.
 * To exit the program write quit.
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

#define MAX_INPUT 255

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr,"usage: %s <receiver ip> <reciver port>\n",argv[0]);
		exit(1);
	}
	int sockfd,
	    err,
	    sendBytes;
	struct addrinfo hints,
			*servInfo,
			*p;
	char sendBuf[MAX_INPUT];
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((err = getaddrinfo(argv[1],argv[2],&hints,&servInfo)) < 0)
	{
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(err));
		exit(1);
	}
	for (p = servInfo; p != NULL; p = p->ai_next)
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
	freeaddrinfo(servInfo);
	if (p == NULL)
	{
		fprintf(stderr,"connect failed\n");
		exit(1);
	}
	p = NULL;
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
		if ((sendBytes = send(sockfd,sendBuf,strlen(sendBuf) + 1,0)) < 0)
		{
			perror("send");
		}
		else
		{
			printf("Sendt message %s, which is %d bytes on socket %d.\n",sendBuf,sendBytes,sockfd);
		}
	}
	printf("Ends the program.\n");
	close(sockfd);
	exit(0);
}
