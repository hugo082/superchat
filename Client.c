//
//  Client.c
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#include "Client.h"

struct Socket* init_connection_to(const char *address) {
    printf("Connecting on %s...\n",address);
    struct Socket *sock = malloc(sizeof(struct Socket));
    sock->sock = socket(AF_INET, SOCK_STREAM, 0);
    sock->sockaddr_in = (SOCKADDR_IN){0};
    struct hostent *hostinfo;
    
    if(sock->sock == INVALID_SOCKET) {
        perror("socket()");
        exit(errno);
    }
    
    hostinfo = gethostbyname(address);
    if (hostinfo == NULL) {
        fprintf (stderr, "Unknown host %s.\n", address);
        exit(EXIT_FAILURE);
    }
    
    sock->sockaddr_in.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    sock->sockaddr_in.sin_port = htons(PORT);
    sock->sockaddr_in.sin_family = AF_INET;
    
    if(connect(sock->sock,(SOCKADDR *) &sock->sockaddr_in, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()");
        exit(errno);
    }
    return sock;
}

int client_command(struct Socket sock, const char *buffer) {
    if (strcmp(buffer, "") == 0)
        return 1;
    size_t size;
    char** tokens = strsplit(buffer, " ", &size);
    if (strcmp(tokens[0],"close") == 0) {
        return 0;
    } else {
        write_buffer(sock, buffer);
        printf("\e[0;32mToi\e[0m : %s\n", buffer);
    }
    fflush(stdout);
    buffer = NULL;
    return 1;
}


void launch_client(const char *address, const char *name) {
    int state = 1;
    struct Socket *sock = init_connection_to(address);
    char buffer[BUF_SIZE];
    fd_set rdfs;
    
    /* send our name */
    write_buffer(*sock, name);
    printf("\e[0;32mConnected\e[0m as %s !\n", name);
    while(state)
    {
        FD_ZERO(&rdfs);
        
        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);
        
        /* add the socket */
        FD_SET(sock->sock, &rdfs);
        
        if(select(sock->sock + 1, &rdfs, NULL, NULL, NULL) == -1) {
            perror("select()");
            exit(errno);
        }
        
        /* something from standard input : i.e keyboard */
        if(FD_ISSET(STDIN_FILENO, &rdfs)) {
            fgets(buffer, BUF_SIZE - 1, stdin);
            {
                char *p = NULL;
                p = strstr(buffer, "\n");
                if(p != NULL)
                    *p = 0;
                else
                    buffer[BUF_SIZE - 1] = 0;
            }
            state = client_command(*sock, buffer);
        }
        else if(FD_ISSET(sock->sock, &rdfs)) {
            int n = read_buffer(*sock, buffer);
            if(n == 0) {
                printf("\e[0;31mServer disconnected !\e[0m\n");
                break;
            }
            puts(buffer);
        }
    }
    
    end_connection(*sock);
    printf("Connection \e[0;32mclosed\e[0m.\n");
}
