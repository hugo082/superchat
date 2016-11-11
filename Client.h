//
//  Client.h
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#ifndef Client_h
#define Client_h

#include "Core.h"

/*
 Init connection to address.
 **/
struct Socket* init_connection_to(const char *address);

/*
 Check if buffer is a client command and execute them.
 **/
int client_command(struct Socket sock, const char *buffer);

/*
 Launch client.
 **/
void launch_client(const char *address, const char *name);

#endif /* Client_h */
