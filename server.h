#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "fifo.h"

int minReceiverThreads;
int maxReceiverThreads;
int currentReceiverThreads;

int minWorkerThreads;
int maxWorkerThreads;
int currentWorkerThreads;

int minMessagesPerReceiverThread = 10;
int desiredMessagesPerReceiverThread = 100;
int minMessagesPerWorkerThread = 10;
int desiredMessagesPerWorkerThread = 50;

// Time in seconds.
int timeBetweenBalancing = 10;
int timeBetweenChecks = 60;

pthread_t *receiverThreads;
pthread_t *workerThreads;
pthread_t checkerThread;

// Socket being used.
int fd;

// Set if the server must continue running or not
int ok = 1;

/**
 * Threads's configuration.
 */
int threadSetup();

/**
 * Worker's threads initial execution function.
 */
void *goWorkerThreads();

/**
 * Receiver's threads initial execution function.
 */
void *goReceiverThreads();

/**
 * Initializes the threads. First of all, the computer's  current available cpus are
 * counted. This is information is then used to setup the threads count (worker and
 * receiver ones, always keeping the receiver/worker = 0.5.
 */
int initializeThreads();

int finalizeThreads();

/**
 * Checks the system load average. We try to catch the three first values but we
 * currently uses just till the higher one the system can tell us about.
 */
int checkLoad();

/**
 * Balance threads.
 */
int balanceThreads();

int setReceiverThreads( unsigned int threadsCount );

int setWorkerThreads( unsigned int threadsCount );

/**
 * Create the UDP socket (socket, bind). Return -1 on error and and positive 
 * integer (the socket's port) if ok.
 */
int initializeSocket();

int finalizeSocket();

int loadData();

int saveData();

int initializeServer();

int finalizeServer();

static void signalHandler( int signal );

int setupSignalHandler();

#include "common.h"
#include "message.h"
#include "message_type.h"
#include "data.h"

#endif
