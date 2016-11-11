//
//  Core.c
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#include "Core.h"

char** strsplit(const char* str, const char* delim, size_t* numtokens) {
    char *s = strdup(str);
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char*));
    char *token, *strtok_ctx;
    for (token = strtok_r(s, delim, &strtok_ctx);
         token != NULL;
         token = strtok_r(NULL, delim, &strtok_ctx)) {
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char*));
        }
        tokens[tokens_used++] = strdup(token);
    }
    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        tokens = realloc(tokens, tokens_used * sizeof(char*));
    }
    *numtokens = tokens_used;
    free(s);
    return tokens;
}

void init(void) {
#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if(err < 0)
    {
        puts("WSAStartup failed !");
        exit(EXIT_FAILURE);
    }
#endif
}


void end(void) {
#ifdef WIN32
    WSACleanup();
#endif
}

void end_connection(struct Socket sock) {
    closesocket(sock.sock);
}

int read_buffer(struct Socket sock, char *buffer) {
    ssize_t n = 0;
    if((n = recv(sock.sock, buffer, BUF_SIZE - 1, 0)) < 0) {
        perror("recv()");
        n = 0;
    }
    buffer[n] = 0;
    return (int)n;
}

void write_buffer(struct Socket sock, const char *buffer) {
    if(send(sock.sock, buffer, strlen(buffer), 0) < 0) {
        perror("send()");
        exit(errno);
    }
}




