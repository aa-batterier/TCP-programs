/*ownTCPSelectProxy.c: Own tcp proxy which utilize select().
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define MAXBUF 1024
#define BACKLOG 50

void *get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	fd_set master, read_fds;
	socklen_t addrlen;
	int fdmax, clientSockfd, newClientSockfd, serverSockfd, rv, i, sendbytes, recvbytes, yes = 0;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage remoteAddr;
	char *remoteBuf, *clientIP;
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
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
		if ((clientSockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) <  0)
		{
			perror("clientSockfd: socket");
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
	FD_SET(clientSockfd, &master);
	fdmax = clientSockfd;
	for (;;)
	{
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0)
		{
			perror("select");
			exit(1);
		}
		for (i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &read_fds))
			{
				if (clientSockfd == i)
				{
					addrlen = sizeof(remoteAddr);
					if ((newClientSockfd = accept(clientSockfd, (struct sockaddr*)&remoteAddr, &addrlen)) < 0)
					{
						perror("accept");
					}
					else
					{
						FD_SET(newClientSockfd, &master);
						if (fdmax < newClientSockfd)
						{
							fdmax = newClientSockfd;
						}
						if ((clientIP = (char*)calloc(1, INET6_ADDRSTRLEN)) == NULL)
						{
							perror("clientIP: realloc");
							exit(1);
						}
						if (inet_ntop(remoteAddr.ss_family, get_in_addr((struct sockaddr*)&remoteAddr), clientIP, INET6_ADDRSTRLEN) == NULL)
						{
							perror("clientIP: inet_ntop");
							exit(1);
						}
						if ((clientIP = (char*)realloc(clientIP, strlen(clientIP) + 1)) == NULL)
						{
							perror("clientIP: realloc");
							exit(1);
						}
						printf("new connection from %s on socket %d\n", clientIP, newClientSockfd);
					}
				}
				else
				{
					if ((remoteBuf = (char*)calloc(1, MAXBUF)) == NULL)
					{
						perror("remoteBuf: calloc");
						exit(1);
					}
					if ((recvbytes = recv(i, remoteBuf, MAXBUF, 0)) <= 0)
					{
						if (recvbytes == 0)
						{
							printf("socket %d hung up\n", i);
						}
						else
						{
							perror("recv");
						}
						close(i);
						FD_CLR(i, &master);
					}
					else
					{
						printf("recived %d bytes from %s on socket %d\n", recvbytes, clientIP, newClientSockfd);
						if ((remoteBuf = (char*)realloc(remoteBuf, strlen(remoteBuf) + 1)) == NULL)
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
						printf("sendt %d bytes to %s on socket %d\n", sendbytes, argv[1], serverSockfd);
						if ((remoteBuf = (char*)realloc(remoteBuf, MAXBUF)) == NULL)
						{
							perror("remoteBuf: realloc");
							exit(1);
						}
						if ((recvbytes = recv(serverSockfd, remoteBuf, MAXBUF, 0)) <= 0)
						{
							if (recvbytes == 0)
							{
								printf("socket %d hung up\n", serverSockfd);
							}
							else
							{
								perror("recv");
							}
						}
						else
						{
							printf("recived %d bytes from %s on socket %d\n", recvbytes, argv[1], serverSockfd);
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
							printf("sendt %d bytes to %s on socket %d\n", sendbytes, argv[1], serverSockfd);
						}
						close(serverSockfd);
						free(remoteBuf);
					}
				}
				free(clientIP);
			}
		}
	}
	close(newClientSockfd);
	close(clientSockfd);
	exit(0);
}
