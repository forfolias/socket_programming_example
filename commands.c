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

void execCommand(long client_socket, char *temp, struct user *user1) {
    char *results;
    char command[BUFFSIZE];
    char com_arg[BUFFSIZE];

    /* split temp to the actual command and the argument if any */
    sscanf(temp, "%s %s", command, com_arg);

    results = (char *) malloc(BUFFSIZE * sizeof(char));
    if (!results) {
        fprintf(stderr, "Memory allocation failed");
        strncpy(results, "Server side error", BUFFSIZE);
    }

    /* empty the results strings on each command */
    memset(results, 0, sizeof(results));

    if      (strcmp(command, "cd"    ) == 0) cd(results, com_arg);
    else if (strcmp(command, "rm"    ) == 0) rm(results, com_arg, user1->id);
    else if (strcmp(command, "pwd"   ) == 0) pwd(results);
    else if (strcmp(command, "uid"   ) == 0) uid(results, user1->id);
    else if (strcmp(command, "get"   ) == 0) sendFile(client_socket, com_arg, results);
    else if (strcmp(command, "read"  ) == 0) sendFile(client_socket, com_arg, results);
    else if (strcmp(command, "help"  ) == 0) help(results);
    else if (strcmp(command, "exit"  ) == 0) strncpy(results, "Closing connection", BUFFSIZE);
    else if (strcmp(command, "list"  ) == 0) list(results);
    else if (strcmp(command, "make"  ) == 0) makefile(results, com_arg, user1->id);
    else if (strcmp(command, "mkdir" ) == 0) makedir(results, com_arg, user1->id);
    else if (strcmp(command, "credit") == 0) credit(results);
    else {
        strncpy(results, "Command not found : ", BUFFSIZE);
        strncat(results, command, BUFFSIZE-strlen(results));
    }

    results[BUFFSIZE-1]='\0';           /* safety first */
    send(client_socket, results, BUFFSIZE, 0 );   /* send the results to client */

    free(results);   /* free some memory */
}

void cd(char *results, char *com_arg) {
    if (chdir(com_arg) == -1) {        /* try to change working directory */
        if (errno == ENOTDIR) strncpy(results, "Not a directory", BUFFSIZE);
        if (errno == ENOENT) strncpy(results, "No such file or directory", BUFFSIZE);
    } else {
        strncpy(results, "Changed directory successful", BUFFSIZE);
    }
}

void makedir(char *results, char *com_arg, int id) {  /* create a directory */
    if (id != 0) {   /* check premisions */
        strncpy(results, "You are not authorized", BUFFSIZE);
    } else {
        if (mkdir(com_arg, (S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
            if (errno == EEXIST) strncpy(results, "Directory already exist", BUFFSIZE);
        } else {
            strncpy(results, "Directory created successful", BUFFSIZE);
        }
    }
}

void makefile(char *results, char *com_arg, int id) { /* create a file */
    FILE *f;

    if (id != 0) {   /* check premisions */
        strncpy(results, "You are not authorized", BUFFSIZE);
    } else {
        f = fopen(com_arg, "r");     /* try to open the file */
        if (!f) {
            f = fopen(com_arg, "w"); /* create the file */
            strncpy(results, "File created successful", BUFFSIZE);
        } else {
            strncpy(results, "File already exist.", BUFFSIZE);
        }
        fclose(f);
    }
}

void rm(char *results, char *com_arg, int id) {  /* remove a file */
    if (id != 0) {     /* check premisions */
        strncpy(results, "You are not authorized", BUFFSIZE);
    } else {
        if (remove(com_arg) == -1) {
            if (errno == ENOENT) strncpy(results, "No such file or directory.", BUFFSIZE);
            if (errno == ENOTEMPTY) strncpy(results, "The directory is not empty.", BUFFSIZE);
        } else {
            strncpy(results, "File removed successful", BUFFSIZE);
        }
    }
}

void pwd(char *results) {         /* print working directory */
    char bigBuffer[PATH_MAX+1];

    if (getcwd(bigBuffer, PATH_MAX+1) == NULL) {
        strncpy(results, "Server side error", BUFFSIZE);
    } else {
        strncpy(results, bigBuffer, BUFFSIZE);
    }
}

void uid(char *results, int id) {  /* print the current user id */
    sprintf(results, "%d", id);
}

int dir_filter(const struct direct *entry) {  /* need by list func */
    return 1;
}

void list(char *results) {
    struct direct **files;
    int nfiles, x;

    char bigBuffer[PATH_MAX+1]="";

    /* number of files at the working dir */
    if ((nfiles = scandir(".", &files, dir_filter, alphasort))==-1) {
        strncpy(results, "Server side error", BUFFSIZE);
    }

    for (x=0;x<nfiles;x++) {           /* for all the files of the directory */
        strncat(bigBuffer, files[x]->d_name, PATH_MAX+1);
        if (files[x]->d_type == DT_DIR)          /* if the file is directory */
            strncat(bigBuffer, "/", PATH_MAX+1);       /* put / next to name */
        strncat(bigBuffer, " ", PATH_MAX+1);          /* space between files */
    }
    strncpy(results, bigBuffer, BUFFSIZE);
}

void credit(char *results) {
    strncpy(results, "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 3 as published by the Free Software Foundation.\nCreated by George Vasilakos. All rights reserved.", BUFFSIZE);
}

void help(char *results) {
    strncpy(results, "\nCommand   : Action\n", BUFFSIZE);
    strncat(results, "help      : Show this help\n", BUFFSIZE-strlen(results));
    strncat(results, "credit    : Show credit information\n", BUFFSIZE-strlen(results));
    strncat(results, "cd dir    : Change working Directory\n", BUFFSIZE-strlen(results));
    strncat(results, "list      : List the files of the working directory\n",BUFFSIZE-strlen(results));
    strncat(results, "pwd       : Print the working directory\n", BUFFSIZE-strlen(results));
    strncat(results, "uid       : Print the current user ID\n", BUFFSIZE-strlen(results));
    strncat(results, "read file : Print text file\n", BUFFSIZE-strlen(results));
    strncat(results, "mkdir dir : Create folder\n", BUFFSIZE-strlen(results));
    strncat(results, "make file : Create file\n", BUFFSIZE-strlen(results));
    strncat(results, "rm file   : Remove file\n", BUFFSIZE-strlen(results));
    strncat(results, "get file  : Send a text file to client\n", BUFFSIZE-strlen(results));
    strncat(results, "clear     : Clear the screen\n", BUFFSIZE-strlen(results));
    strncat(results, "exit      : Close the connection with server\n", BUFFSIZE-strlen(results));
}

void sendFile(long socket, char *filename, char *results) {
    FILE *f;
    char c[2];

    f = fopen(filename, "r");
    if (!f) {
        strncpy(results, "No such file or directory.", BUFFSIZE);
        c[0] = EOF;
        send(socket, c, sizeof(char), 0); /* send EOF to stop client from receiving */
    } else {
        do {                    /* start sending */
            c[0] = fgetc(f);    /* read a character from file */
            send(socket, c, sizeof(char), 0);    /* send the character to client */
        } while (c[0] != EOF);  /* till the end of file */
        fclose(f);              /* close file */
        strncpy(results, "File transmited successful.", BUFFSIZE);
    }
}
