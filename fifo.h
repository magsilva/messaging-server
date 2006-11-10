#ifndef FIFO_H
#define FIFO_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "common.h"


struct fifo_t {
	struct message_t *head;
	struct message_t *foot;
	unsigned int size;
	unsigned char preparedForDestruction;
	pthread_mutex_t mutex;
	pthread_cond_t empty;
};

// Message's queue
struct fifo_t messages;

#include "message.h"

int fifo_init( struct fifo_t *fifo );

int fifo_prepare_destroy( struct fifo_t *fifo );

int fifo_wait_empty( struct fifo_t *fifo );

int fifo_destroy( struct fifo_t *fifo );

int fifo_put( struct fifo_t *fifo, struct message_t *message );

struct message_t *fifo_get( struct fifo_t *fifo );

unsigned int fifo_size( struct fifo_t *fifo );

#endif
