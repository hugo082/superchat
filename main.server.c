//
//  main.server.c
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#include "Server.h"

int main() {
    init();
    launch_server();
    end();
    return EXIT_SUCCESS;
}
