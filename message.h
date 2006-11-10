#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"
#include "fifo.h"
#include "crc16.h"
#include "data.h"

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <unistd.h>

#define MAX_TRIES 3



/**
* from - identificador do usuário que enviou a mensagem.
* to - identificador do usuário que receberá (ou recebeu) a mensagem.
* type - tipo de mensagem
* argumentsSize - tamanho dos argumentos da mensagem
* arguments - argumentos da mensagem
* next - ponteiro para a próxima mensage
*/
struct message_t {
	unsigned int from;
	unsigned int to;
	unsigned int type;
	unsigned int messageId;
	unsigned short sequence;
	unsigned int argumentsSize;
	unsigned short crc16;
	char *arguments;
	struct message_t *next;
	struct sockaddr address;
};

unsigned int messageHeaderSize();

unsigned int messageSize( struct message_t *message );

int freeMessage( struct message_t *message );

char *buildBuffer( struct message_t *message );

struct message_t *buildMessage( char *buffer );

int sendMessage( int fd, struct sockaddr *to, struct message_t *message, struct fifo_t *messages );

struct message_t *receiveMessage( int fd );

int sendAcknowledgeMessage( int fd, struct message_t *message );

#endif
