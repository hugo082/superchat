//
//  Server.h
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#ifndef Server_h
#define Server_h

#include "Core.h"

#define MAX_CLIENTS 100

typedef struct
{
    struct Socket sock;
    char name[BUF_SIZE];
}Client;

typedef struct
{
    int state, nbClients, max;
    struct Socket *sock;
    char buffer[BUF_SIZE];
    Client clients[MAX_CLIENTS];
}Server;

/*
 Create a new server.
 **/
Server* init_server();

/*
 Init connection at PORT
 **/
struct Socket* init_connection(void);

/*
 Remove all clients.
 **/
void clear_clients(Client *clients, int actual);

/*
 Remove a client.
 Client must be disconnected.
 **/
void remove_client(Client *clients, int to_remove, int *actual);

/*
 Send buffer at all clients.
 **/
void send_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, int from_server);

/*
 Launch the server.
 **/
void launch_server(void);

/*
 Check if buffer is a server command and execute them.
 **/
int server_command(Client *clients, const char *buffer, int *actual);

#endif /* Server_h */
