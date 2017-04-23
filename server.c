/**
 Copyright (C) George Vasilakos 2009 <forfolias@linuxteam.cs.teilar.gr>

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 3 as published
 by the Free Software Foundation.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "network.h"
#include "commands.h"

#include <pthread.h>

int login(long);
int accept_connection(int);
void getCommand(long, struct user *);
void *handle_client(void *);
int auth(struct user *);

int main(int argc,char *argv[]) {
    struct sockaddr_in  server_address;
    uint16_t port = ADRESS_PORT;
    pthread_t thread;
    long clnt_sock;
    FILE *f;
    int c;

    while ((c = getopt (argc, argv, "hp:")) != -1) {
        switch (c) {
        case 'p':
            port = atoi(optarg);
            if (port == 0) {
                fprintf(stderr, "Not a valid port number\n");
                port = ADRESS_PORT;
            }
            break;
        case 'h':
        default :
            printf("Usage : %s [-p port]\n", argv[0]);
            return 0;
        }
    }

    printf("Using port : %d\n", port);

    f = fopen(ACCESS_LIST, "r");

    if (!f) {
        fprintf(stderr, "Can't find access list file\n");
        return 1;
    } else {
        fclose(f);
    }

    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    /* Give the socket a name. */
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons (port);

    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(sock, MAXPENDING) < 0 ) {
        fprintf(stderr, "listen() failed");
    }

    while (1) {
        printf("Wating for clients.\n");
        clnt_sock = accept_connection(sock);
        if (clnt_sock < 0) {
            printf("Error accept failed");
            close(sock);
            return -1;
        } else {
            if (pthread_create(&thread, NULL, handle_client, (void *) clnt_sock)) {
                close(sock);           /* error creating the thread, so close the socket */
            }
        }
    }
    return 0;
}


int login(long client_socket) {
    char *strings[]={"Enter username : ",
                     "Enter password : ",
                     "Authentication failed\n",
                     "Login successful.\n"};
    struct user user1;
    char buffer[BUFFSIZE];

    /* Send ask for username to client */
    send(client_socket, strings[0], strlen(strings[0]), 0);

    /* Receive username from client */
    if (!read(client_socket, buffer, sizeof(buffer)) <= 0)
        strncpy(user1.name, buffer, BUFFSIZE);

    /* Send ask for password to client */
    send(client_socket, strings[1], strlen(strings[1]), 0 );

    /* Receive password from client */
    if (!read(client_socket, buffer, sizeof(buffer)) <= 0)
        strncpy(user1.pass, buffer, BUFFSIZE);

    /* Send the results of authentication to client */
    if (auth(&user1)) {
        send(client_socket, strings[2], strlen(strings[2]), 0 );
        return 1;    /* auth fail */
    } else {
        send(client_socket, strings[3], strlen(strings[3]), 0 );  /* auth was ok */
        getCommand(client_socket, &user1);           /* start receiving commands */
        return 0;
    }
}


int accept_connection(int server_socket) {
    int client_socket;                          /* Socket descriptor for client */
    struct sockaddr_in client_address;          /* Client address */
    unsigned int client_length;                 /* Length of client address data structure */

    /* Set the size of the in-out parameter */
    client_length = sizeof(client_address);

    /* Wait for a client to connect */
    if ((client_socket=accept(server_socket,(struct sockaddr *) &client_address,&client_length)) <0){
        fprintf(stderr, "accept() failed");
    }

    /* client_socket is connected to a client! */
    printf("Handling client : %s\n", inet_ntoa(client_address.sin_addr));

    return client_socket;
}


void *handle_client (void *arg) {
    long client_socket;
    int attemp=1;

    client_socket = (long) arg;

    while (login(client_socket) && attemp<MAXATTEMPTS) attemp++;
    close(client_socket);     /* close socket if auth fail or exit command */
    pthread_exit(0);  /* terminate the thread if auth fail or exit command */
}


int auth(struct user *user1) {
    FILE *file;
    char buffer[200];
    char user_read[BUFFSIZE];
    char pass_read[BUFFSIZE];
    int uid_read;

    file = fopen(ACCESS_LIST, "r");

    /* read lines from the access file */
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (buffer[0]!='#' && buffer[0]!='\n') {             /* skip comments and blank lines */
            sscanf(buffer, "%s %s %d", user_read, pass_read, &uid_read);    /* split the line */
            if ((strcmp(user1->name, user_read) == 0) && (strcmp(user1->pass, pass_read) == 0)) {
                user1->id = uid_read;   /* user found, store his user id (permisions) */
                fclose(file);
                return 0;
            }
        }
    }
    fclose(file);
    return 1;
}


void getCommand(long client_socket, struct user *user1) {
    char buffer[BUFFSIZE];

    do {     /* start receiving commands */
        if (!read(client_socket, buffer, sizeof(buffer)) <= 0)
            execCommand(client_socket, buffer, user1); /* if there wasn't error, execute the command */
    } while (strcmp(buffer, "exit") != 0);          /* if the command is exit, then close connection */
    close(client_socket);
    printf("Client dissconected\n");
}
