//
//  ServerCommand.h
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#ifndef ServerCommand_h
#define ServerCommand_h

#include "Server.h"

/*
 Remove client with name
 **/
void remove_client_with_name(Client *clients, char *name, int *actual);

/*
 List all clients.
 **/
void ls_client(Client *clients, int *actual);

#endif /* ServerCommand_h */
