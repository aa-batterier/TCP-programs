/*
 * Program which sets up a UDP server.
 * The server can only receive messages from the connected client.
 * The program takes one argument and that is the port which the server shall listen to.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_INPUT 255

void *get_in_addr(struct sockaddr *sa)
{
        if (sa->sa_family == AF_INET)
        {
                return &(((struct sockaddr_in*)sa)->sin_addr);
        }
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc,char *argv[])
{
        if (argc != 2)
        {
                fprintf(stderr,"usage %s <port>\n",argv[0]);
                exit(1);
        }
        int rv,
            sockfd,
            recvBytes;
        socklen_t addrLen;
        struct sockaddr_storage remoteAddr;
        struct addrinfo hints,
                        *servInfo,
                        *p;
        char *remoteIP,
             recvBuf[MAX_INPUT];
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
                if ((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) < 0)
                {
                        perror("socket");
                        continue;
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
        for (;;)
        {
                memset(recvBuf,0,MAX_INPUT);
                addrLen = sizeof(remoteAddr);
                if ((recvBytes = recvfrom(sockfd,recvBuf,MAX_INPUT,0,(struct sockaddr*)&remoteAddr,&addrLen)) < 0)
                {
                        perror("recvfrom");
                        continue;
                }
                remoteIP = (char*)malloc(INET6_ADDRSTRLEN);
                if (inet_ntop(remoteAddr.ss_family,get_in_addr((struct sockaddr*)&remoteAddr),remoteIP,INET6_ADDRSTRLEN) == NULL)
                {
                        perror("inet_ntop");
                        free(remoteIP);
                        continue;
                }
                remoteIP = (char*)realloc(remoteIP,strlen(remoteIP)+1);
		printf("Received message %s, which is %d bytes on socket %d, from ip %s on port %s.\n",recvBuf,recvBytes,sockfd,remoteIP,argv[1]);
                free(remoteIP);
        }
        close(sockfd);
        exit(0);
}
