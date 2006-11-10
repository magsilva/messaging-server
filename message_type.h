#ifndef MESSAGE_TYPE_H
#define MESSAGE_TYPE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "message.h"

int messageType0( int fd, struct message_t *incomingMessage );

int messageType1( int fd, struct message_t *incomingMessage );

int messageType2( int fd, struct message_t *incomingMessage );

int messageType3( int fd, struct message_t *incomingMessage );

int messageType4( int fd, struct message_t *incomingMessage );

int messageType5( int fd, struct message_t *incomingMessage );

int messageType6( int fd, struct message_t *incomingMessage );

int messageType7( int fd, struct message_t *incomingMessage );

int messageType8( int fd, struct message_t *incomingMessage );

int messageType9( int fd, struct message_t *incomingMessage );

#endif
