/**
 Copyright (C) George Vasilakos 2009 <forfolias@linuxteam.cs.teilar.gr>

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 3 as published
 by the Free Software Foundation.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "network.h"

#include <signal.h>

int login(int);
void recvFile(long, char *);
void printFile(long);
void sendCommand(int, struct user *);
int checkFile(char *);
void signalhandle(int);

int main(int argc,char *argv[]) {
    uint16_t port = ADRESS_PORT;
    char server_IP[50]=ADRESS_IP;
    struct sockaddr_in  server_address;
    int attemp=1;
    int c;

    while ((c = getopt (argc, argv, "hp:i:")) != -1) {
        switch (c) {
        case 'p':
            port = atoi(optarg);
            if (port == 0) {
                fprintf(stderr, "Not a valid port number\n");
                port = ADRESS_PORT;
            }
            break;
        case 'i':
            strncpy(server_IP, optarg, sizeof(server_IP));
            break;
        case 'h':
        default :
            printf("Usage : %s [-p port] [-i hostname | ip]\n", argv[0]);
            return 0;
        }
    }

    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (sock < 0) {
        perror ("socket");
        return 1;
    }

    /* Give the socket a name. */
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons (port);

    /* Resolve the hostname of the server */
    struct hostent *host = gethostbyname(server_IP);
    if (host == NULL) {
        if (h_errno == HOST_NOT_FOUND) {
            fprintf(stderr, "No such host is known in the database.\n");
        } else if (h_errno == TRY_AGAIN) {
                fprintf(stderr, "If you try again later, you may succeed then.\n");
        } else if (h_errno == NO_ADDRESS) {
                fprintf(stderr, "The host database contains an entry for the name,");
                fprintf(stderr, "but it doesn't have an associated Internet address.\n");
        } else {
                fprintf(stderr, "Unknown error while resolving hostname.\n");
        }
        return 1;
    } else {
        memcpy(&server_address.sin_addr, host->h_addr, host->h_length);
    }

    printf("Using host : %s\n", server_IP);
    printf("Using port : %d\n", port);

    /* Establish the connection to the server */
    if (connect(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "connect() failed\n");
        return 1;
    }

    signal(SIGINT, signalhandle);  /* handle the interupt signal ^C */
    signal(SIGQUIT, signalhandle);     /* handle the quit signal ^\ */

    while (login(sock) && attemp<MAXATTEMPTS) attemp++;

    close(sock);

    return 0;
}

int login(int sock) {
    char buffer[BUFFSIZE]="";
    struct user user1;

    /* Receive asking for username */
    if (!read (sock, buffer, sizeof(buffer)) <= 0) printf("%s", buffer);
    fgets(user1.name, sizeof(user1.name), stdin);

    /* remove \n at the end of the string */
    user1.name[strlen(user1.name)-1]='\0';

    /* Send username to server */
    send(sock, user1.name, sizeof(user1.name), 0 );

    /* Receive asking for password */;
    if (!read (sock, buffer, sizeof(buffer)) <= 0) printf("%s", buffer);
    fgets(user1.pass, sizeof(user1.pass), stdin);
    user1.pass[strlen(user1.pass)-1]='\0';

    /* Send password to server */
    send(sock, user1.pass, sizeof(user1.pass), 0 );

    /* Receive and print the results of login */;
    if (!read (sock, buffer, sizeof(buffer)) <= 0) printf("%s", buffer);

    /* On successful login, continue to the command prompt */
    if (strcmp(buffer, "Login successful.\n") == 0) {
        sendCommand(sock, &user1);
        return 0;
    } else {
        return 1;
    }
}

void recvFile(long socket, char *filename) {
    FILE *f;
    char c[2];

    f = fopen("file_received", "r");         /* try to open the file */
    if (!f) {
        f = fopen(filename, "w");            /* create the file */
        do {                                 /* start receiving */
            read(socket, c, sizeof(char));   /* read a character from the socket */
            if (c[0] == EOF) break;          /* skip EOF, fclose will do the job */
            fwrite(c, sizeof(char), 1, f);   /* write the character to file */
        } while (1);                         /* continue receive till EOF */
        fclose(f);                           /* close file */
    } else {
        fclose(f);                           /* file exist and open, close it */
    }
}

void sendCommand(int sock, struct user *user1) {
    char buffer[BUFFSIZE];
    char *results;
    char command[BUFFSIZE];
    char com_arg[BUFFSIZE];

    results = (char *) malloc(BUFFSIZE * sizeof(char));
    if (!results) {
        fprintf(stderr, "Memory allocation failed");
        return;
    }

    /* start sending the commands to server */
    do {
        strncpy(buffer,  "", BUFFSIZE);          /* empty the buffer  string on each command */
        strncpy(command, "", BUFFSIZE);          /* empty the command string on each command */
        strncpy(com_arg, "", BUFFSIZE);          /* empty the com_arg string on each command */
        memset(results, 0, sizeof(results));     /* empty the results string on each command */

        printf("%s@server $ ", user1->name);     /* show the command prompt */

        fgets(buffer, sizeof(buffer), stdin);    /* get the command */
        buffer[strlen(buffer)-1]='\0';           /* safety first */
        sscanf(buffer, "%s %s", command, com_arg);   /* split buffer to the actual command   */
                                                     /* and the argument if there is any     */
        if (strcmp(command, "clear") == 0) {
            system("clear");                         /* clear the screen */
        } else
            if (strcmp(command, "get") == 0) {       /* if the command is get */
                if (!checkFile(com_arg)) {           /* ckeck if the file already exist */
                    send(sock, buffer, sizeof(command), 0 );        /* send the command */
                    recvFile(sock, com_arg);                /* start receiving the file */
                    if (!read (sock, results, BUFFSIZE) <= 0)          /* no read error */
                        printf("%s\n", results);                /* so print the results */
                } else {
                    printf("File already exist, skipping command.\n");
                }
            } else
                if (strcmp(command, "read") == 0) {
                    send(sock, buffer, sizeof(command), 0 );            /* send the command */
                    printFile(sock);                            /* start receiving the file */
                    if (!read(sock, results, BUFFSIZE) <= 0) /* if no error reading results */
                        printf("%s\n", results);                       /* print the results */
                } else
                    if (strcmp(command, "") == 0) {        /* if just hit enter, do nothing */
                        ;
                    } else {  /* if get or read, we have already send the command to server */
                        if (strcmp(command, "get") != 0 && strcmp(command, "read") != 0)
                            send(sock, buffer, sizeof(command), 0 );
                        if (!read (sock, results, BUFFSIZE) <= 0)  /* read results of other commands */
                            printf("%s\n", results);               /* print the results */
                    }
    } while (strcmp(buffer, "exit") != 0);    /* stop asking for a new command if user typed exit */

    free(results);
}

void printFile(long socket) {
    char c[2];

    do {                                 /* start receiving */
        read(socket, c, sizeof(char));   /* read a character from the socket */
        if (c[0] == EOF) break;          /* skip EOF */
        fputc(c[0], stdout);             /* print character to stdout */
    } while (1);                         /* continue receive till EOF */
    printf("\n");
}

void signalhandle(int sig) { /* if user hit ^C or ^\ don't exit and crash the server */
    signal(sig, SIG_IGN);              /* just ignore the signal and print a message */
    printf("\nTo terminate the client type the command exit\n");
    signal(sig, signalhandle);                       /* reinstall the signal handler */
}

int checkFile(char *filename) {
    FILE *f;

    f = fopen(filename, "r");
    if (!f) {              /* file doesn't exist */
        return 0;
    } else {               /* file exist */
        fclose(f);
        return 1;
    }
}
