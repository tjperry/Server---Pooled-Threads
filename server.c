#include "cs537.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

/* TO DO LIST 
 *	fix code to work with 4 arguments, not 2              (DONE)
 *	create the number of threads specified by argv[2]     (DONE)
 *	create buffer with the size specified by argv[3]      (DONE)
 *	implement consumer & producer CVs in regards to...    (IN PROGRESS)
 *		accepted connection being allocated to a thread
 *		producer logic underway.. review in morning, I'm tired
 *	
 *////////////////////////////////////////////////////////////////
/* QUESTIONS
 *	
 *	
 *
 *////////////////////////////////////////////////////////////////

// Function Declaration
void *consumeConn();
void getargs(int *port, int *threadCnt, int argc, char *argv[]);

// Global Variables
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

int threadsAvail;

int *connBufferPushPtr;
int *connBufferPullPtr;
long int connBufferSize;
long int connBufferStart;

// CS537: Parse the new arguments too
void getargs(int *port, int *threadCnt, int argc, char *argv[])
{
	if (argc != 4) {
	fprintf(stderr, "Usage: %s <port> <threads> <buffers>\n", argv[0]);
	exit(1);
	}

	*port = atoi(argv[1]);
	*threadCnt = atoi(argv[2]);
	threadsAvail  = atoi(argv[2]);
	connBufferSize = atoi(argv[3]);
	int connBuffer[connBufferSize];
	connBufferPushPtr = connBuffer;
	connBufferPullPtr = connBuffer;
	connBufferStart = (long int)connBufferPushPtr;

	int i;
	for( i = 0; i < (connBufferSize); i++ ){
		connBuffer[i] = 0;
	}
}

void *consumeConn(){
	int thConnfd = 0;
	
	while(1){		
		pthread_mutex_lock(&lock);
		while(*connBufferPullPtr == 0)
			pthread_cond_wait(&full, &lock);

		thConnfd = *connBufferPullPtr;
		*connBufferPullPtr = 0;
		connBufferPullPtr = (int *)(connBufferStart + ((((long int)connBufferPullPtr - connBufferStart) + 1) % (connBufferSize)));
		pthread_cond_signal(&empty);

		requestHandle(thConnfd);
		Close(thConnfd);
		pthread_mutex_unlock(&lock);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int listenfd, connfd, port, clientlen, threadCnt;
	struct sockaddr_in clientaddr;

	getargs(&port, &threadCnt, argc, argv);

	// 
	// CS537: Create some threads...
	//
//	pthread_t p[threadCnt];
//	int i;
//	for(i = 0; i < threadCnt; i++ ){
//		pthread_create(&(p[i]), NULL, consumeConn, NULL);
//	}

	listenfd = Open_listenfd(port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

		// 
		// CS537: In general, don't handle the request in the main thread.
		// Save the relevant info in a buffer and have one of the worker threads 
		// do the work.
		// 
		pthread_mutex_lock(&lock);
		while(*connBufferPushPtr != 0)
			pthread_cond_wait(&empty, &lock);
		
		*connBufferPushPtr = connfd;
		connBufferPushPtr = (int *)(connBufferStart + ((((long int)connBufferPushPtr - connBufferStart) + 1) % (connBufferSize)));
		fprintf(stderr, "%p = %ld + (((%p - %ld) + 1) mod (%ld))", connBufferPushPtr, connBufferStart, connBufferPushPtr, connBufferStart, connBufferSize);
		pthread_cond_signal(&full);
		pthread_mutex_unlock(&lock); 

	}

}


    


 
