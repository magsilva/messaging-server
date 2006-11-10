#include "fifo.h"

/**
* Initilializes the fifo structure.
*/
int fifo_init( struct fifo_t *fifo ) {
	if ( fifo == NULL ) {
		perror( "\nfifo_init" );
		return -1;
	}
	fifo->head = NULL;
	fifo->foot = NULL;
	fifo->size = 0;
	fifo->preparedForDestruction = 0;
	if ( pthread_mutex_init( &fifo->mutex, NULL ) == -1 ) {
		perror( "\nfifo_init" );
		return -1;
	}
	if ( pthread_cond_init( &fifo->empty, NULL ) == -1 ) {
		perror( "\nfifo_init" );
		return -1;
	}	;
	return 1;
}

/**
 * Prepare fifo for destruction. Disallow put() and doesn't block
 * on behalf of conditional variable at get().
 */
int fifo_prepare_destroy( struct fifo_t *fifo ) {
	if ( fifo == NULL ) {
		return -1;
	}
	
	pthread_mutex_lock( &fifo->mutex );

	fifo->preparedForDestruction = 1;

	pthread_mutex_unlock( &fifo->mutex );
	pthread_cond_broadcast( &fifo->empty );
	return 1;
}

/**
* Free the fifo's resource.
*/
int fifo_destroy( struct fifo_t *fifo ) {
	struct message_t *current;

	if ( fifo == NULL ) {
		return -1;
	}

	pthread_mutex_lock( &fifo->mutex );
	while ( fifo->head != NULL ) {
		current = fifo->head;
		fifo->head = fifo->head->next;
		freeMessage( current );
	}
	fifo->foot = NULL;
	pthread_mutex_unlock( &fifo->mutex );
	pthread_mutex_destroy( &fifo->mutex );
	pthread_cond_destroy( &fifo->empty );
	return 1;
}

/**
 * Stop till the fifo is empty.
 */
int fifo_wait_empty_on_destruction( struct fifo_t *fifo ) {
	if ( fifo == NULL ) {
		return -1;
	}
	
	pthread_mutex_lock( &fifo->mutex );
	while ( fifo->head != NULL && fifo->preparedForDestruction == 0 ) {
		pthread_cond_wait( &fifo->empty, &fifo->mutex );
	}
	pthread_mutex_unlock( &fifo->mutex );
	return 1;
}


/**
* Adds a message to the fifo.
*/
int fifo_put( struct fifo_t *fifo, struct message_t *message ) {

	if ( fifo == NULL || message == NULL ) {
		return -1;
	}
	
	pthread_mutex_lock( &fifo->mutex );

	// Cannot save messages in a fifo being destructed.
	if ( fifo->preparedForDestruction == 1 ) {
		return -1;
	}
	
	if ( fifo->foot == NULL ) {
		fifo->head = message;
		fifo->foot = message;
		fifo->foot->next = NULL;
		fifo->size++;
		pthread_mutex_unlock( &fifo->mutex );
		pthread_cond_signal( &fifo->empty );
		return 1;
	}
	fifo->foot = message;
	fifo->foot->next = NULL;
	fifo->size++;
	pthread_mutex_unlock( &fifo->mutex );
	return 1;
}

/**
* Take a message from the fifo.
*/
struct message_t *fifo_get( struct fifo_t *fifo ) {
	struct message_t *result = NULL;

	if ( fifo == NULL ) {
		return NULL;
	}
	
	pthread_mutex_lock( &fifo->mutex );

	// There's no condition to wait when being destroyed.
	while ( fifo->head == NULL && fifo->preparedForDestruction == 0 ) {
		pthread_cond_wait( &fifo->empty, &fifo->mutex );
 	}
	result = fifo->head;
	if ( fifo->head != NULL ) {
		fifo->head = fifo->head->next;
	} else {
		fifo->foot = NULL;

		// Wake up anyone that is waiting of an empty fifo.
		if ( fifo->preparedForDestruction == 1 ) {
			pthread_cond_broadcast( &fifo->empty );
		}
	}
	fifo->size--;
	pthread_mutex_unlock( &fifo->mutex );
	return result;
}

unsigned int fifo_size( struct fifo_t *fifo ) {
	int result = 0;

	if ( fifo == NULL ) {
		return 0;
	}

	pthread_mutex_lock( &fifo->mutex );
	result = fifo->size;
	pthread_mutex_unlock( &fifo->mutex );
	return result;
}
