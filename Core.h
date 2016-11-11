//
//  Core.h
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#ifndef Core_h
#define Core_h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

struct Socket {
    SOCKET sock;
    SOCKADDR_IN sockaddr_in;
    SOCKADDR sockaddr;
    IN_ADDR in_addr;
};

#define CRLF		"\r\n"
#define PORT	 	1977

#define BUF_SIZE	1024

/*
 Split une string dans un array.
 **/
char** strsplit(const char* str, const char* delim, size_t* numtokens);

/*
 Init.
 **/
void init(void);

/*
 End.
 **/
void end(void);

/*
 Close socket connection.
 **/
void end_connection(struct Socket sock);

/*
 Read socket buffer.
 **/
int read_buffer(struct Socket sock, char *buffer);

/*
 Write to socket.
 **/
void write_buffer(struct Socket sock, const char *buffer);

#endif /* Core_h */
