#include "server.h"

/**
 * Threads's configuration.
 */
int threadSetup() {
	sigset_t signal;
				
	// The cancel state can be enabled or disabled.
	if ( pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL ) != 0 ) {
		perror( "threadSetup" );
		return -1;
	}

	// The cancel tyoe can be asyncronous (instantaneous) or deffered (at
	// cancellation points).
	if ( pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL ) != 0 ) {
		perror( "threadSetup" );
		return -1;
	}

	sigfillset( &signal );
	pthread_sigmask( SIG_SETMASK, &signal, 0 );	
	return 1;
}

/**
 * Checker thread initial execution function.
 */
void *goCheckerThread() {
	int result = 0;
	struct user_t *current;
	struct message_t nullMessage;
	
	nullMessage.type = 0;
	nullMessage.argumentsSize = 0;
					
	if ( threadSetup() == -1 ) {
		result = -1;
		pthread_exit( &result );
	}


	while ( 1 ) {
		pthread_testcancel();
		sleep( timeBetweenChecks );
		
		current = users;
		while ( current != NULL ) {

			// Check if user is not offline. If no ack, set it as offline.
			if ( current->status != 0 ) {
				// Enviar pacote null com sendMessage().
				result = sendMessage( fd, current->address, &nullMessage, &messages );
				// Se o usuário não respondeu, provavelmente ficou offline e esqueceu de
				//  dar logoff
				if ( result == -1 ) {
					user_logoff( current->id );
				}
			}
			current = current->next;
		}
		
	}
}

/**
 * Worker's threads initial execution function.
 */
void *goWorkerThreads() {
	int result;
	struct user_t *user;

	if ( threadSetup() == -1 ) {
		result = -1;
		pthread_exit( &result );
	}
	while ( 1 ) {
		struct message_t *message = fifo_get( &messages );
		if ( message == NULL ) {
			result = -1;
			pthread_exit( &result );
		}

		switch ( message->type ) {
			case 0:
				messageType0( fd, message );
				break;
			case 1:
				messageType1( fd, message );
				break;
			case 2:
				messageType2( fd, message );
				break;
			case 3:
				messageType3( fd, message );
				break;
			case 4:
				messageType4( fd, message );
				break;
			case 5:
				messageType5( fd, message );
				break;
			case 6:
				messageType6( fd, message );
				break;
			case 7:
				messageType7( fd, message );
				break;
			case 8:
				messageType8( fd, message );
				break;
			case 9:
				messageType9( fd, message );
				break;
		default:
#ifdef DEBUG
			printf( "Unknown message type: %d ", message->type );
#endif
		}
		freeMessage( message );
	}
}

/**
 * Receiver's threads initial execution function.
 */
void *goReceiverThreads() {
	struct message_t *message;
	pthread_t id;
	unsigned long good = 0, bad = 0;

	id	= pthread_self();
	if ( threadSetup() == -1 ) {
			bad = -1;
			pthread_exit( &bad );
	}
	while ( 1 ) {
		message = receiveMessage( fd );
		if ( message == NULL ) {
			bad++;
		} else {
			fifo_put( &messages, message );
			good++;
		} 
#ifdef DEBUG
		printf( "\n\t<%lx> %lx packages received (%lx/%lx)", id, good + bad, good, bad );
#endif
	}
}


/**
 * Initializes the threads. First of all, the computer's  current available cpus are
 * counted. This is information is then used to setup the threads count (worker and
 * receiver ones, always keeping the receiver/worker = 0.5.
 */
int initializeThreads() {
	int i = 0;
	// Number of CPUs currently online
	long cpuCount = sysconf( _SC_NPROCESSORS_ONLN );

	
	switch ( cpuCount ) {
		case 0:
			printf( "\n_SC_NPROCESSORS_ONLN option not available" );
			return -1;
		case -1:
			printf( "\nsysconf" );
			return -1;
	}

	// Calculating some limits to threads to live in the server.
	minReceiverThreads = 1;
	maxReceiverThreads = cpuCount;
	minWorkerThreads = 2 * minReceiverThreads;
	maxWorkerThreads = 2 * maxReceiverThreads;

	// Setup receiver threads.
	currentReceiverThreads = minReceiverThreads;
	receiverThreads = (pthread_t *)calloc( currentReceiverThreads, sizeof( pthread_t ) );
	if ( receiverThreads == NULL ) {
		perror( "\ninitializeThreads:receiverThreads" );
		return -1;
	}
	for ( i = 0; i < currentReceiverThreads; i++ ) {
		if ( pthread_create( &receiverThreads[ i ], NULL, goReceiverThreads, NULL ) == -1 ) {
			perror( "\ninitializeThreads:receiverThreads" );
			return -1;
		}
	}

	// Setup worker threads.
	currentWorkerThreads = minWorkerThreads;
	workerThreads = (pthread_t *)calloc( currentWorkerThreads, sizeof( pthread_t ) );
	if ( workerThreads == NULL ) {
		perror( "\ninitializeThreads:workerThreads" );
		return -1;
	}
	for ( i = 0; i < currentWorkerThreads; i++ ) {
		if ( pthread_create( &workerThreads[ i ], NULL, goWorkerThreads, NULL ) == -1 ) {
			perror( "\ninitializeThreads:workerThreads" );
			return -1;
		}
	}

	// Setup checker thread.
	if ( pthread_create( &checkerThread, NULL, goCheckerThread, NULL ) == -1 ) {
		perror( "\ninitializarThreads:checkerThread" );
		return -1;
	}
	
#ifdef DEBUG
	printf( "\nThreads initialized: \n\tReceiver threads: %d (%d/%d)\n\tWorker threads: %d (%d/%d)", currentReceiverThreads, minReceiverThreads, maxReceiverThreads, currentWorkerThreads, minWorkerThreads, maxWorkerThreads );
#endif

	return 1;
}

/**
* Stops all threads, kill them all (but gently).
*/
int finalizeThreads() {
	// Terminate the receiver threads.
	setReceiverThreads( 0 );

	// Wait till the message's queue is empty, then terminate the worker threads.
	fifo_wait_empty_on_destruction( &messages );
	setWorkerThreads( 0 );

	// Terminate the checker thread.
	pthread_cancel( checkerThread );
	pthread_join( checkerThread, NULL );

	return 1;
}

/**
* Changes the receiver's thread in the system.
*/
int setReceiverThreads( unsigned int threadsCount ) {
	int i = 0;

	// If no change, just return.
	if ( threadsCount == currentReceiverThreads ) {
		return 0;
	}

	// If must decrease the threads allocated, just reduce "currentReceiverThreads"  and
	// stop the exceeding threads ( currentReceiverThreads - threadsCount ).
	if ( currentReceiverThreads >= threadsCount )  {
		for ( i = currentReceiverThreads; i > threadsCount; i-- ) {
			pthread_cancel( receiverThreads[ i - 1 ] );
		}
		for ( i = currentReceiverThreads; i > threadsCount; i-- ) {
			pthread_join( receiverThreads[ i - 1 ], NULL );
		}
	// Else (must increase the threads allocated), creates a new struct, copy the ones from the current
	// receiverThreads to it, free the old structure and create the new threads as needed.
	} else {
		receiverThreads = (pthread_t *)realloc( receiverThreads, threadsCount * sizeof( pthread_t ) );
		for ( i = currentReceiverThreads; i < threadsCount; i++ ) {
			if ( pthread_create( &receiverThreads[ i ], NULL, goReceiverThreads, NULL ) == -1 ) {
				perror( "\nsetReceiverThreads" );
				return -1;
			}
		}
	}
#ifdef DEBUG
	printf( "\nChanged from %d to %d receiver threads", currentReceiverThreads, threadsCount );
#endif
	currentReceiverThreads = threadsCount;
	return 1;
}

/**
* Changes the worker threads in the system.
*/
int setWorkerThreads( unsigned int threadsCount ) {
	int i = 0;

	if ( threadsCount == currentWorkerThreads ) {
		return 0;
	}
	
	if ( currentWorkerThreads >= threadsCount ) {
		for ( i = currentWorkerThreads; i > threadsCount; i-- ) {
			pthread_cancel( workerThreads[ i - 1 ] );
		}
		for ( i = currentWorkerThreads; i > threadsCount; i-- ) {
			pthread_join( workerThreads[ i - 1 ], NULL );
		}
	} else {
		workerThreads = (pthread_t *)realloc( workerThreads, threadsCount * sizeof( pthread_t ) );
		for ( i = currentWorkerThreads; i < threadsCount; i++ ) {
			if ( pthread_create( &workerThreads[ i ], NULL, goWorkerThreads, NULL ) == -1 ) {
				perror( "\nsetWorkerThreads" );
				return -1;
			}
		}
	}
#ifdef DEBUG
	printf( "\nChanged from %d to %d worker threads", currentWorkerThreads, threadsCount );
#endif
	currentWorkerThreads = threadsCount;
	return 1;
}

/**
 * Balance threads. Always keeps threads count between respectives min and max limits.
 * Increase if too many messages to be worked out, decrease if too few. Increasing rate
 * is always faster, decreasing not.
 */
int balanceThreads() {
	unsigned int fifoSize = 0;
	unsigned int messagesPerReceiver;
	unsigned int messagesPerWorker;
	unsigned int threadsCount = 0;

	fifoSize = fifo_size( &messages );
	messagesPerReceiver = fifoSize / currentReceiverThreads;
	messagesPerWorker = fifoSize / currentWorkerThreads;

	// Receiver threads balancing
	if ( messagesPerReceiver > desiredMessagesPerReceiverThread ) {
		threadsCount = fifoSize / desiredMessagesPerReceiverThread;
		threadsCount = ( threadsCount >= maxReceiverThreads ? maxReceiverThreads : threadsCount );
		setReceiverThreads( threadsCount );
	} else {
		if ( messagesPerReceiver < minMessagesPerReceiverThread ) {
	  	threadsCount = currentReceiverThreads - 1;
			threadsCount = ( threadsCount <= minReceiverThreads ? minReceiverThreads : threadsCount );
			setReceiverThreads( threadsCount );
		}
	}

	// Worker threads balancing
	if ( messagesPerWorker > desiredMessagesPerWorkerThread ) {
		threadsCount = fifoSize / desiredMessagesPerWorkerThread;
		threadsCount = ( threadsCount >= maxWorkerThreads ? maxWorkerThreads : threadsCount );
		setWorkerThreads( threadsCount );
	} else {
		if ( messagesPerWorker < minMessagesPerWorkerThread ) {
			threadsCount = currentWorkerThreads - 1;
			threadsCount = ( threadsCount <= minWorkerThreads ? minWorkerThreads : threadsCount);
			setWorkerThreads( threadsCount );
		}
	}
	return 1;
}

/**
 * Create the UDP socket (socket, bind). Return -1 on error and and positive
 * integer (the socket's port) if ok.
 */
int initializeSocket() {
	int length;
	struct sockaddr_in server;
	uint16_t port;

	fd = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 ) {
		perror( "\nSocket creation not possible" );
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl( INADDR_ANY );
	server.sin_port = htons( SERVER_PORT );
	if ( bind( fd, (struct sockaddr *)&server, sizeof( server ) ) == -1 ) {
		perror( "\nSocket binding not possible" );
		return -1;
	}
	length = sizeof( server );
	if ( getsockname( fd, (struct sockaddr *)&server, &length ) == -1 ) {
		perror( "\nCould not take socket" );
	}

	port = ntohs( server.sin_port );
#ifdef DEBUG
	printf( "\nServer port: %d", port );
#endif
	return port;
}

/**
* Closes the UDP socket.
*/
int finalizeSocket() {
	close( fd );
	return 1;
}

#ifdef DEBUG
/**
* Helper function, loads the data from a file to the server's data structure.
*/
int loadData() {
	return 1;
}

/**
* Helper function, saves the data from the server's data structure to a file.
*/
int saveData() {
	return 1;
}
#endif

/**
* Initialize the server, allocating the needed data structure .
*/
int initializeServer() {
	printf( "\nMessage server - Version %s", VERSION );
	initializeSocket();
	fifo_init( &messages );
#ifdef DEBUG
	loadData();
#endif
	initializeThreads();
	setupSignalHandler();
	printf( "\nServer data initialized, start serving" );
	return 1;
}

/**
* Finishe the server, freeing all the data structures used.
*/
int finalizeServer() {
	printf( "\nClosing server, please wait..." );
	fifo_prepare_destroy( &messages );
	finalizeThreads();
	fifo_destroy( &messages );
	finalizeSocket();
#ifdef DEBUG
	saveData();
#endif
	printf( "\nService finished. Have a nice day." );
	return 1;
}

/**
 * Gerencia os sinais.
 */
int setupSignalHandler() {
	struct sigaction signalAction;

	signalAction.sa_handler = signalHandler;
	signalAction.sa_flags = 0;
	sigfillset( &signalAction.sa_mask );

	sigdelset( &signalAction.sa_mask, SIGINT );
	sigdelset( &signalAction.sa_mask, SIGHUP );
	sigdelset( &signalAction.sa_mask, SIGQUIT );
	sigdelset( &signalAction.sa_mask, SIGTERM );
	
	sigaction( SIGINT, &signalAction, 0 );
	sigaction( SIGHUP, &signalAction, 0 );
	sigaction( SIGQUIT, &signalAction, 0 );
	sigaction( SIGTERM, &signalAction, 0 );
}

/**
 * Disable all signal handlers.
 */
static void signalHandler( int signal ) {
#ifdef DEBUG
	if ( ok == 1 ) {
		printf( "\nAcknowledged service termination user's request." );
	} else {
		printf( "\nAlready finishing service, please don't be so impatient. Thank you." );
	}
#endif
	ok = 0;
}

/**
 * Checks the system load average. We try to catch the three first values but we
 * currently uses just till the higher one the system can tell us about.
 */
int checkLoad() {
	int elementsCount = 3;
	double loadAverage[ elementsCount ];
	int i = 0;
	time_t currentTime;
	struct tm *localTime;

	elementsCount = getloadavg( loadAverage, elementsCount );
	if ( elementsCount == -1 ) {
		perror( "\ncheckLoad" );
		return -1;
	}
#ifdef DEBUG
	// Get the current time.
	currentTime = time( NULL );

	/* Convert it to local time representation. */
	localTime = localtime( &currentTime );

	printf( "\nSystem load average: %d/%d/%d %d:%d:%d    \t", localTime->tm_mday, 1 + localTime->tm_mon, 1900 + localTime->tm_year, localTime->tm_hour, localTime->tm_min, localTime->tm_sec );
	for ( i = 0; i < elementsCount; i++ ) {
		printf( " %.2f ", loadAverage[ i ] );
	}
#endif
	return 1;
}

int main() {
	initializeServer();
	while ( ok ) {
		checkLoad();
		balanceThreads();
		sleep( timeBetweenBalancing );
	}
	finalizeServer();
	printf( "\n" );
	return 0;
}
