/*
 * This program sends an insta message to a server over TCP.
 * After the message has been sent the program ends.
 * The program takes three arguments.
 * The first argument is the ip address of the server.
 * The second argument is the port of the server.
 * The third argument is the message which the program will send to the server.
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
	if (argc != 4)
	{
		fprintf(stderr,"usage: %s <receiver ip> <reciver port> <\"message\">\n",argv[0]);
		exit(1);
	}
	int sockfd,
	    rv,
	    sendbytes;
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
	if ((sendbytes = send(sockfd,argv[3],strlen(argv[3]) + 1,0)) < 0)
	{
		perror("send");
		close(sockfd);
		exit(1);
	}
	printf("Sendt message %s, which is %d bytes on socket %d.\n",argv[3],sendbytes,sockfd);
	close(sockfd);
	exit(0);
}
