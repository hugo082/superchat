//
//  ServerCommand.c
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#include "ServerCommand.h"

void remove_client_with_name(Client *clients, char *name, int *actual) {
    for (int i = 0; i < *actual; i++) {
        if (strcmp(clients[i].name, name) == 0) {
            closesocket(clients[i].sock.sock);
            remove_client(clients, i, actual);
            printf("Client %s \e[0;32mremoved\e[0m.\n", name);
            fflush(stdout);
            return;
        }
    }
    printf("Client %s \e[1;31mnot found\e[0m.\n", name);
    fflush(stdout);
}

void ls_client(Client *clients, int *actual) {
    printf("+------ START\n");
    for (int i = 0; i < *actual; i++) {
        printf("| %s\n", clients[i].name);
    }
    if (*actual < 1)
        printf("| \e[1;33mEmpty\e[0m\n");
    printf("+------ END\n");
    fflush(stdout);
}
