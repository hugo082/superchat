//
//  main.client.c
//  superchat
//
//  Created by Hugo on 11/11/2016.
//  Copyright © 2016 hfqt. All rights reserved.
//

#include "Client.h"

int main(int argc, char **argv) {
    init();
    if(argc < 2) {
        printf("Usage : %s [address] [pseudo]\n", argv[0]);
        return EXIT_FAILURE;
    }
    launch_client(argv[1], argv[2]);
    end();
    printf("\e[0;36mGood bye %s.\n\e[0m", argv[2]);
    return EXIT_SUCCESS;
}
