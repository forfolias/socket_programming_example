/**
 Copyright (C) George Vasilakos 2009 <forfolias@linuxteam.cs.teilar.gr>

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 3 as published
 by the Free Software Foundation.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>

void execCommand(long, char *, struct user *);
void list(char *);
void help(char *);
void credit(char *);
void cd(char *, char *);
void makedir(char *, char *, int);
void makefile(char *, char *, int);
void pwd(char *);
void uid(char *, int);
void rm(char *, char *, int);
void sendFile(long, char *, char *);

#endif // COMMANDS_H_INCLUDED
