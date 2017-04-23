AM_CFLAGS = -Wall
SUBDIRS = data
bin_PROGRAMS = server client
server_SOURCES = server.c commands.c commands.h network.h
server_LDFLAGS = -lpthread
client_SOURCES = client.c network.h