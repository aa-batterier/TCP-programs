/*ownTCPForkProxy.c: Own tcp proxy which utilize fork().
 *Copyright (C) 2016  Andreas Johansson
 *This program is free software: you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

#define MAXBUF 1024
#define BACKLOG 50

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	pid_t pid;
	int clientSockfd, newClientSockfd, serverSockfd, rv, sendbytes, recvbytes, yes = 0;
	char *clientIP, *remoteBuf;
	socklen_t addrlen;
	struct sockaddr_storage remoteAddr;
	struct addrinfo hints, *servinfo, *p;
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, argv[2], &hints, &servinfo)) < 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((clientSockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		{
			perror("socket");
			continue;
		}
		if (setsockopt(clientSockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
		{
			perror("setsockopt");
			exit(1);
		}
		if (bind(clientSockfd, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(clientSockfd);
			perror("bind");
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	if (p == NULL)
	{
		fprintf(stderr, "bind failed\n");
		exit(1);
	}
	p = NULL;
	if (listen(clientSockfd, BACKLOG) < 0)
	{
		perror("listen");
		exit(1);
	}
	for (;;)
	{
		addrlen = sizeof(remoteAddr);
		if ((newClientSockfd = accept(clientSockfd, (struct sockaddr*)&remoteAddr, &addrlen)) < 0)
		{
			perror("accept");
			exit(1);
		}
		if ((clientIP = (char*)calloc(1, INET6_ADDRSTRLEN)) == NULL)
		{
			perror("clientIP: calloc");
			exit(1);
		}
		if (inet_ntop(remoteAddr.ss_family, get_in_addr((struct sockaddr*)&remoteAddr), clientIP, INET6_ADDRSTRLEN) == NULL)
		{
			perror("inet_ntop");
			exit(1);
		}
		if ((clientIP = (char*)realloc(clientIP, strlen(clientIP) + 1)) == NULL)
		{
			perror("clientIP: realloc");
			exit(1);
		}
		printf("new connection from %s on socket %d\n", clientIP, newClientSockfd);
		if ((pid = fork()) < 0)
		{
			perror("fork");
			exit(1);
		}
		/*The child handels request from the client and forwards them to the server.
		 *Receives data from the server and forwards it to the client.
		 *Because the sockets are initiate before fork() the child inheritets them.*/
		else if (pid == 0)
		{
			close(clientSockfd); /*We don't need this socket anymore.*/
			if ((remoteBuf = (char*)calloc(1, MAXBUF)) == NULL)
			{
				perror("remoteBuf: calloc");
				exit(1);
			}
			if ((recvbytes = recv(newClientSockfd, remoteBuf, MAXBUF, 0)) <= 0)
			{
				if (recvbytes == 0)
				{
					fprintf(stderr, "%s on socket %d hung up\n", clientIP, newClientSockfd);
				}
				else
				{
					perror("recv");
					exit(1);
				}
				close(newClientSockfd);
			}
			else
			{
				printf("recived %d bytes from %s on socket %d\n", recvbytes, clientIP, newClientSockfd);
				if ((remoteBuf = realloc(remoteBuf, strlen(remoteBuf) + 1)) == NULL)
				{
					perror("remoteBuf: realloc");
					exit(1);
				}
				memset(&hints, 0, sizeof(hints));
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) < 0)
				{
					fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
					exit(1);
				}
				for (p = servinfo; p != NULL; p = p->ai_next)
				{
					if ((serverSockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
					{
						perror("socket");
						continue;
					}
					if (connect(serverSockfd, p->ai_addr, p->ai_addrlen) < 0)
					{
						close(serverSockfd);
						perror("connect");
						continue;
					}
					break;
				}
				freeaddrinfo(servinfo);
				if (p == NULL)
				{
					fprintf(stderr, "connect failed\n");
					exit(1);
				}
				p = NULL;
				if ((sendbytes = send(serverSockfd, remoteBuf, strlen(remoteBuf) + 1, 0)) < 0)
				{
					perror("send");
					exit(1);
				}
				printf("sendt %d bytes to %s\n", sendbytes, argv[1]);
				if ((remoteBuf = (char*)realloc(remoteBuf, MAXBUF)) == NULL)
				{
					perror("remoteBuf: realloc");
					exit(1);
				}
				//memset(&remoteBuf, 0, MAXBUF);
				if ((recvbytes = recv(serverSockfd, remoteBuf, MAXBUF, 0)) <= 0)
				{
					if (recvbytes == 0)
					{
						fprintf(stderr, "socket %d hung up\n", serverSockfd);
					}
					else
					{
						perror("recv");
						exit(1);
					}
					close(serverSockfd);
				}
				else
				{
					if ((remoteBuf = (char*)realloc(remoteBuf, strlen(remoteBuf) + 1)) == NULL)
					{
						perror("remoteBuf: realloc");
						exit(1);
					}
					if ((sendbytes = send(newClientSockfd, remoteBuf, strlen(remoteBuf) + 1, 0)) < 0)
					{
						perror("send");
						exit(1);
					}
					printf("sendt %d bytes to %s\n", sendbytes, clientIP);
				}
				close(serverSockfd);
			}
			free(remoteBuf);
			exit(0);
		}
		free(clientIP);
		close(newClientSockfd);
		if (waitpid(pid, NULL, 0) != pid)
		{
			perror("waitpid");
			exit(1);
		}
	}
	exit(0);
}
