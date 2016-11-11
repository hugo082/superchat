#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "server.h"
#include "client.h"


char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;
    
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }
    count += last_comma < (a_str + strlen(a_str) - 1);
    count++;
    result = malloc(sizeof(char*) * count);
    
    if (result) {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);
        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }
    
    return result;
}


static void init(void)
{
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

void remove_client_with_name(Client *clients, char *name, int *actual) {
    for (int i = 0; i < *actual; i++) {
        if (strcmp(clients[i].name, name) == 0) {
            closesocket(clients[i].sock);
            remove_client(clients, i, actual);
            printf("Client %s \e[0;32mremoved\e[0m.", name);
            fflush(stdout);
            return;
        }
    }
    printf("Client %s \e[1;31mnot found\e[0m.", name);
    fflush(stdout);
}

int command(Client *clients, char *buffer, int *actual) {
    if (strcmp(buffer, "") == 0)
        return 1;
    char** tokens = str_split(buffer, ' ');
    if (strcmp(tokens[0],"close") == 0) {
        return 0;
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

static void end(void) {
#ifdef WIN32
    WSACleanup();
#endif
}

static void app(void) {
    int state = 1;
    SOCKET sock = init_connection();
    char buffer[BUF_SIZE];
    /* the index for the array */
    int actual = 0;
    int max = sock;
    /* an array for all clients */
    Client clients[MAX_CLIENTS];
    
    fd_set rdfs;
    
    printf("Server \e[0;32mstarted\e[0m.\n");
    while(state)
    {
        int i = 0;
        FD_ZERO(&rdfs);
        
        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);
        
        /* add the connection socket */
        FD_SET(sock, &rdfs);
        
        /* add socket of each client */
        for(i = 0; i < actual; i++)
        {
            FD_SET(clients[i].sock, &rdfs);
        }
        
        if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }
        
        /* something from standard input : i.e keyboard */
        if(FD_ISSET(STDIN_FILENO, &rdfs))
        {
            fgets(buffer, BUF_SIZE - 1, stdin);
            {
                char *p = NULL;
                p = strstr(buffer, "\n");
                if(p != NULL)
                    *p = 0;
                else
                    buffer[BUF_SIZE - 1] = 0;
            }
            state = command(clients, buffer, &actual);
        }
        else if(FD_ISSET(sock, &rdfs))
        {
            /* new client */
            SOCKADDR_IN csin = { 0 };
            size_t sinsize = sizeof csin;
            int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
            if(csock == SOCKET_ERROR)
            {
                perror("accept()");
                continue;
            }
            
            /* after connecting the client sends its name */
            if(read_client(csock, buffer) == -1)
            {
                /* disconnected */
                continue;
            }
            
            /* what is the new maximum fd ? */
            max = csock > max ? csock : max;
            
            FD_SET(csock, &rdfs);
            
            Client c = { csock };
            strncpy(c.name, buffer, BUF_SIZE - 1);
            clients[actual] = c;
            actual++;
            printf("Client %s connected !\n", c.name);
        }
        else
        {
            int i = 0;
            for(i = 0; i < actual; i++)
            {
                /* a client is talking */
                if(FD_ISSET(clients[i].sock, &rdfs))
                {
                    Client client = clients[i];
                    int c = read_client(clients[i].sock, buffer);
                    /* client disconnected */
                    if(c == 0)
                    {
                        closesocket(clients[i].sock);
                        remove_client(clients, i, &actual);
                        strncpy(buffer, client.name, BUF_SIZE - 1);
                        strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                        send_message_to_all_clients(clients, client, actual, buffer, 1);
                        printf("Client %s \e[1;33mdisconnected\e[0m !\n", client.name);
                    }
                    else
                    {
                        printf("Superchat %s : %s\n", client.name, buffer);
                        send_message_to_all_clients(clients, client, actual, buffer, 0);
                    }
                    break;
                }
            }
        }
    }
    
    clear_clients(clients, actual);
    end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
    int i = 0;
    for(i = 0; i < actual; i++)
    {
        closesocket(clients[i].sock);
    }
    printf("\e[1;33mAll client have been removed.\e[0m\n");
}

static void remove_client(Client *clients, int to_remove, int *actual) {
    memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
    (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server) {
    int i = 0;
    char message[BUF_SIZE + 10];
    message[0] = 0;
    for(i = 0; i < actual; i++)
    {
        /* we don't send message to the sender */
        if(sender.sock != clients[i].sock)
        {
            if(from_server == 0)
            {
                strncpy(message, "\e[0;35m", 10);
                strncat(message, sender.name, BUF_SIZE - 1);
                strncat(message, "\e[0m : ", sizeof message - strlen(message) - 1);
            }
            strncat(message, buffer, sizeof message - strlen(message) - 1);
            write_client(clients[i].sock, message);
        }
    }
}

static int init_connection(void) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin = { 0 };
    
    if(sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }
    
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;
    
    //end_connection(sock);
    
    if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
    {
        perror("bind()");
        exit(errno);
    }
    
    if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }
    printf("Server \e[0;32mcreated\e[0m.\n");
    
    return sock;
}

static void end_connection(int sock) {
    closesocket(sock);
    printf("Server \e[0;32mclosed\e[0m.\n");
}

static int read_client(SOCKET sock, char *buffer) {
    int n = 0;
    if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
        perror("recv()");
        n = 0;
    }
    buffer[n] = 0;
    return n;
}

static void write_client(SOCKET sock, const char *buffer) {
    if(send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("send()");
        exit(errno);
    }
}

int main(int argc, char **argv)
{
    init();
    
    app();
    
    end();
    
    return EXIT_SUCCESS;
}
