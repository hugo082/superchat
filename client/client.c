nclude <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "client.h"

void client(const char *address, const char *name)
{
	SOCKET sock = init_connection(address);
	char buffer[BUF_SIZE];
	fd_set rdfs;
	write_server(sock, name);
	while(1)
	{
		FD_ZERO(&rdfs);
		FD_SET(STDIN_FILENO, &rdfs);
		FD_SET(sock, &rdfs);
		if(select(sock + 1, &rdfs, NULL, NULL, NULL) == -1)
		{
			perror("select()");
			exit(errno);
		}
		if(FD_ISSET(STDIN_FILENO, &rdfs))
		{
			fgets(buffer, BUF_SIZE - 1, stdin);
			{
				char *p = NULL;
				p = strstr(buffer, "\n");
				if(p != NULL)
				{
					*p = 0;
				}
				else
				{
					buffer[BUF_SIZE - 1] = 0;
				}
			}
			write_server(sock, buffer);
		}
		else if(FD_ISSET(sock, &rdfs))
		{
			int n = read_server(sock, buffer);
			if(n == 0)
			{
				printf("Server disconnected !\n");
				break;
			}
			printf("Server: ");
			puts(buffer);
		}
	}
	end_connection(sock);
}

int init_connection(const char *address)
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN sin = { 0 };
	struct hostent *hostinfo;
	if(sock == INVALID_SOCKET)
	{
		perror("socket()");
		exit(errno);
	}
	hostinfo = gethostbyname(address);
	if (hostinfo == NULL)
	{
		fprintf (stderr, "Unknown host %s.\n", address);
		exit(EXIT_FAILURE);
	}
	sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
	sin.sin_port = htons(PORT);
	sin.sin_family = AF_INET;
	if(connect(sock,(SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		perror("connect()");
		exit(errno);
	}
	return sock;
}

void end_connection(int sock)
{
	closesocket(sock);
}
int read_server(SOCKET sock, char *buffer)
{
	int n = 0;
	if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
	{
		perror("recv()");
		exit(errno);
	}
	buffer[n] = '\0';
	return n;
}
void write_server(SOCKET sock, const char *buffer)
{
	if(send(sock, buffer, strlen(buffer), 0) < 0)
	{
		perror("send()");
		exit(errno);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Usage : %s [address] [pseudo]\n", argv[0]);
		return EXIT_FAILURE;
	}
	client(argv[1], argv[2]);
	return EXIT_SUCCESS;
}

