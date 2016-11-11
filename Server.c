//
//  Server.c
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#include "Server.h"
#include "ServerCommand.h"

Server* init_server() {
    Server *server = malloc(sizeof(Server));
    server->state = 1;
    server->sock = init_connection();
    server->nbClients = 0;
    server->max = server->sock->sock;
    return server;
}

struct Socket* init_connection(void) {
    struct Socket *sock = malloc(sizeof(struct Socket));
    sock->sock = socket(AF_INET, SOCK_STREAM, 0);
    sock->sockaddr_in = (SOCKADDR_IN){0};
    
    if(sock->sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }
    
    sock->sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    sock->sockaddr_in.sin_port = htons(PORT);
    sock->sockaddr_in.sin_family = AF_INET;
    
    //end_connection(sock);
    
    if(bind(sock->sock,(SOCKADDR *) &sock->sockaddr_in, sizeof sock->sockaddr_in) == SOCKET_ERROR)
    {
        perror("bind()");
        exit(errno);
    }
    
    if(listen(sock->sock, MAX_CLIENTS) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }
    printf("Server \e[0;32mcreated\e[0m.\n");
    
    return sock;
}

void launch_server(void) {
    Server *server = init_server();
    
    fd_set rdfs;
    
    printf("Server \e[0;32mstarted\e[0m.\n");
    while(server->state)
    {
        int i = 0;
        FD_ZERO(&rdfs);
        
        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);
        
        /* add the connection socket */
        FD_SET(server->sock->sock, &rdfs);
        
        /* add socket of each client */
        for(i = 0; i < server->nbClients; i++)
        {
            FD_SET(server->clients[i].sock.sock, &rdfs);
        }
        
        if(select(server->max + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }
        
        /* something from standard input : i.e keyboard */
        if(FD_ISSET(STDIN_FILENO, &rdfs))
        {
            fgets(server->buffer, BUF_SIZE - 1, stdin);
            {
                char *p = NULL;
                p = strstr(server->buffer, "\n");
                if(p != NULL)
                    *p = 0;
                else
                    server->buffer[BUF_SIZE - 1] = 0;
            }
            server->state = server_command(server->clients, server->buffer, &(server->nbClients));
        }
        else if(FD_ISSET(server->sock->sock, &rdfs))
        {
            struct Socket *clientSock = malloc(sizeof(struct Socket));
            
            clientSock->sockaddr_in = (SOCKADDR_IN){ 0 };
            clientSock->sockaddr = *(SOCKADDR *)&clientSock->sockaddr_in;
            socklen_t sinsize = sizeof clientSock->sockaddr_in;
            clientSock->sock = accept(server->sock->sock, &clientSock->sockaddr, &sinsize);
            if(clientSock->sock == SOCKET_ERROR) {
                perror("accept()");
                continue;
            }
            
            if(read_buffer(*clientSock, server->buffer) == -1)
                continue; // disconnected
            
            /* what is the new maximum fd ? */
            server->max = clientSock->sock > server->max ? clientSock->sock : server->max;
            
            FD_SET(clientSock->sock, &rdfs);
            
            Client c = { *clientSock };
            strncpy(c.name, server->buffer, BUF_SIZE - 1);
            server->clients[server->nbClients] = c;
            server->nbClients++;
            printf("Client %s connected !\n", c.name);
        }
        else
        {
            int i = 0;
            for(i = 0; i < server->nbClients; i++)
            {
                /* a client is talking */
                if(FD_ISSET(server->clients[i].sock.sock, &rdfs))
                {
                    Client client = server->clients[i];
                    int c = read_buffer(server->clients[i].sock, server->buffer);
                    /* client disconnected */
                    if(c == 0)
                    {
                        closesocket(server->clients[i].sock.sock);
                        remove_client(server->clients, i, &(server->nbClients));
                        strncpy(server->buffer, client.name, BUF_SIZE - 1);
                        strncat(server->buffer, " disconnected !", BUF_SIZE - strlen(server->buffer) - 1);
                        send_to_all_clients(server->clients, client, server->nbClients, server->buffer, 1);
                        printf("Client %s \e[1;33mdisconnected\e[0m !\n", client.name);
                    }
                    else
                    {
                        printf("Superchat %s : %s\n", client.name, server->buffer);
                        send_to_all_clients(server->clients, client, server->nbClients, server->buffer, 0);
                    }
                    break;
                }
            }
        }
    }
    
    clear_clients(server->clients, server->nbClients);
    end_connection(*server->sock);
    printf("Server Connection \e[0;32mclosed\e[0m.\n");
}



void clear_clients(Client *clients, int actual) {
    int i = 0;
    for(i = 0; i < actual; i++) {
        closesocket(clients[i].sock.sock);
    }
    printf("\e[1;33mAll client have been removed.\e[0m\n");
}

void remove_client(Client *clients, int to_remove, int *actual) {
    memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
    (*actual)--;
}

void send_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, int from_server) {
    int i = 0;
    char message[BUF_SIZE + 10];
    message[0] = 0;
    for(i = 0; i < actual; i++) {
        if(sender.sock.sock != clients[i].sock.sock) {
            if(from_server == 0)
            {
                strncpy(message, "\e[0;35m", 10);
                strncat(message, sender.name, BUF_SIZE - 1);
                strncat(message, "\e[0m : ", sizeof message - strlen(message) - 1);
            }
            strncat(message, buffer, sizeof message - strlen(message) - 1);
            write_buffer(clients[i].sock, message);
        }
    }
}

int server_command(Client *clients, const char *buffer, int *actual) {
    if (strcmp(buffer, "") == 0)
        return 1;
    size_t size;
    char** tokens = strsplit(buffer, " ", &size);
    if (strcmp(tokens[0],"close") == 0) {
        return 0;
    } else if (strcmp(tokens[0],"ls") == 0) {
        ls_client(clients, actual);
    } else if (strcmp(tokens[0],"rm") == 0) {
        if (tokens[1] == NULL || strcmp(tokens[1], "") == 0) {
            printf("\e[1;31mMissing name parameter !\e[0m\n");
            fflush(stdout);
        } else
            remove_client_with_name(clients, tokens[1], actual);
    }
    buffer = NULL;
    return 1;
}
