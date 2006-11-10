#ifndef SENDMSG_H
#define SENDMSG__H

#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <unistd.h>

#include "message.h"

	int fd;
	struct addrinfo *res, *res0;
	struct sockaddr_in server;


int sendMsgType0();

int sendMsgType1(unsigned int userID, char *userPassword);

unsigned int sendMsgType2( char *fullname, char *email, char *nick, char *password);

int sendMsgType3( unsigned int myID, unsigned int toID, char *message);

int sendMsgType4( unsigned int myID, unsigned int addID);

int sendMsgType5( unsigned int myID, unsigned int delID );

int sendMsgType6( unsigned int myID );

int sendMsgType7( unsigned int myID );

int sendMsgType8( unsigned int myID );

#endif
