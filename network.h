/**
 Copyright (C) George Vasilakos 2009 <forfolias@linuxteam.cs.teilar.gr>

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 3 as published
 by the Free Software Foundation.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>

#define ADRESS_PORT           12345
#define ADRESS_IP             "127.0.0.1"
#define MAXPENDING            5
#define MAXATTEMPTS           3
#define BUFFSIZE              500
#define ACCESS_LIST           "data/access.dat"

struct user {
    char name[BUFFSIZE];
    char pass[BUFFSIZE];
    int id;
};

#endif // NETWORK_H_INCLUDED
