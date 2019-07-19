/*
 * This program sends the same message a specified amout of times with a specified
 * interval in miliseconds between each message to a server over TCP.
 * The program takes five arguments.
 * The first argument is the ip address of the server.
 * The second argument is the port of the server.
 * The third argument is the message which will be sent.
 * The fourth argument is a number which will be the amount of messages sent to the server.
 * The fifth argument is the interval between each message in miliseconds.
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

int main(int argc, char *argv[])
{
	if (argc != 6)
	{
		fprintf(stderr,"usage: %s <receiver ip> <reciver port> <\"message\"> <number of messages> <interval between messages in ms>\n",argv[0]);
		exit(1);
	}
	int sockfd,
	    rv,
	    sendBytes,
	    numberOfMessages = atoi(argv[4]),
	    interval = atoi(argv[5])*1000,
            // Saves the size of the message so I don't need to calculate it in every loop.
            messageSize = strlen(argv[3])+1;
	struct addrinfo hints,
			*servinfo,
			*p;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(argv[1],argv[2],&hints,&servinfo)) < 0)
	{
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
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
	for (int loops = 0; loops < numberOfMessages; loops++)
	{
		if ((sendBytes = send(sockfd,argv[3],messageSize,0)) < 0)
		{
                        perror("send");
			fprintf(stderr,"send failed on message number %d.\n",loops);
			close(sockfd);
			exit(1);
		}
		printf("Sendt message %s number %d, which is %d bytes on socket %d.\n",argv[3],loops,sendBytes,sockfd);
		usleep(interval);
	}
	close(sockfd);
	exit(0);
}
