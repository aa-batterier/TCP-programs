/*
 * This program sends the same message unlimited amout of times with a specified
 * interval in miliseconds between each message to a server over UDP.
 * The program takes four arguments.
 * The first argument is the ip address of the server.
 * The second argument is the port of the server.
 * The third argument is the message which will be sent.
 * The fourth argument is the interval between each message in miliseconds.
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
	if (argc != 5)
	{
		fprintf(stderr,"usage: %s <receiver ip> <reciver port> <\"message\"> <interval between messages in ms>\n",argv[0]);
		exit(1);
	}
	int sockfd,
	    err,
	    sendBytes,
            interval = atoi(argv[4])*1000,
            // Saves the size of the message so I don't need to calculate it in every loop.
            messageSize = strlen(argv[3])+1;
	struct addrinfo hints,
			*servInfo,
			*p;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
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
                break;
        }
        freeaddrinfo(servInfo);
	if (p == NULL)
	{
		fprintf(stderr,"connection to server failed\n");
                close(sockfd);
		exit(1);
	}
	for (int loops = 0;; loops++)
	{
                if ((sendBytes = sendto(sockfd,argv[3],messageSize,0,p->ai_addr,p->ai_addrlen)) < 0)
                {
                        perror("sendto");
                        fprintf(stderr,"sendto failed on message number %d.\n",loops);
                        p = NULL;
                        close(sockfd);
                        exit(1);
                }
		printf("Sendt message %s number %d, which is %d bytes on socket %d.\n",argv[3],loops,sendBytes,sockfd);
		usleep(interval);
        }
        p = NULL;
        close(sockfd);
        exit(0);
}
