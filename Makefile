## Simple SDL mini code

CC=gcc

# Sources
COMMON_SRCS= Core.c
ALL_SRCS= $(COMMON_SRCS) Client.c Server.c ServerCommand.c

SRCS_MAIN= $(SRCS) main.c
SRCS_CLIENT= $(COMMON_SRCS) Client.c main.client.c
SRCS_SERVER= $(COMMON_SRCS) Server.c ServerCommand.c main.server.c

CFLAGS= -Wall -Wextra -std=c99 -O3

OBJS= $(SRCS_MAIN)

SRC= IMG/Core.c IMG/main.c
OBJ= ${SRC:.c=.o}

all: main

main: ${OBJS}

client: $(SRCS_CLIENT)
	$(CC) $(CFLAGS) $^ -o $@

server: $(SRCS_SERVER)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o
	rm -f main
	rm -f server
	rm -f client

# END