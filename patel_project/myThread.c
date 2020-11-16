/* This is the skeleton program for you to use to create your own
 * threading library based on setcontext, makecontext, swapcontext,
 * getcontext routines. This work must be done on CSE or CSCE. 
 */

/* Enter the names of all team members here:
 * Member 1: Karthik Pagilla
 * Member 2: Utkarsh Hardia
 * Member 3: Soham Patel
 */
#define _XOPEN_SOURCE 600

#include "myThread.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ucontext.h>
#include <signal.h>

#define INTERVAL 200000
#define BOUND 5000000
#define STACKSIZE 8192
#define THREADS 12
#define PRINT 1000000

#define POSIX 1
#define MYSEM 2
#define NONE 0

/* This program can operate in three modes:
 * with POSIX semaphore (define MUTEX POSIX) 
 * with your own semaphore (define MUTEX MYSEM)
 * or without any semaphore (don't define MUTEX at all)
 * */

//#define MUTEX POSIX
#define MUTEX MYSEM
//#define MUTEX NONE

#define DEBUG 1

ucontext_t context[THREADS], myCleanup, myMain;
int status[THREADS];
struct itimerval clocktimer;
int totalThreads = 0;
int currentThread = 0;
int lock_key = 0;
int followUpThread = 0;
volatile int sharedCounter = 0;

#if MUTEX == POSIX
sem_t mutex;
#elif MUTEX == MYSEM
mysem_t mutex;
#endif

int main(void)
{
	char *myStack[THREADS];
	char myCleanupStack[STACKSIZE];
	int j;
	/* initialize timer to send signal every 200 ms */
	clocktimer.it_value.tv_sec = 0;
	clocktimer.it_value.tv_usec = INTERVAL;
	clocktimer.it_interval.tv_sec = 0;
	clocktimer.it_interval.tv_usec = INTERVAL;
	setitimer(ITIMER_REAL, &clocktimer, 0);
	sigset(SIGALRM, signalHandler);

	/* You need to set up an execution context for the cleanup
	 * function to use (I've already created myCleanup for you). 
	 * You need to initialize it to include the runtime stack space
	 * (myCleanupStack), stack size, the context to return to when
	 * cleanup function finishes. Make sure you use the makecontext
	 * command to map the cleanup function to myCleanup context.
	 */

	// set up your cleanup context here.
	getcontext(&myMain);
	getcontext(&myCleanup);
	myCleanup.uc_stack.ss_sp = myCleanupStack;
	myCleanup.uc_stack.ss_size = STACKSIZE;
	myCleanup.uc_link = &myMain;
	makecontext(&myCleanup, cleanup, 0);
	/* Next, you need to set up contexts for the user threads that will run
	 * task1 and task2. We will assign even number threads to task1 and
	 * odd number threads to task2. 
	 */
	for (j = 0; j < THREADS; j++)
	{
		// set up your context for each thread here (e.g., context[0])
		// for thread 0. Make sure you pass the current value of j as
		// the thread id for task1 and task2.
		getcontext(&context[j]);
		myStack[j] = (char *)malloc(STACKSIZE);
		context[j].uc_stack.ss_sp = myStack[j];
		context[j].uc_stack.ss_size = STACKSIZE;
		context[j].uc_link = &myCleanup;

		if (j % 2 == 0)
		{
#if DEBUG == 1
			printf("Creating task1 thread[%d].\n", j);
			makecontext(&context[j], task1, 1, j);
#endif
			// map the corresponding context to task1
		}
		else
		{
#if DEBUG == 1
			printf("Creating task2 thread[%d].\n", j);
			makecontext(&context[j], task2, 1, j);
#endif
			// map the corresponding context to task2
		}

		// you may want to keep the status of each thread using the
		// following array. 1 means ready to execute, 2 means currently
		// executing, 0 means it has finished execution.

		status[j] = 1;

		// You can keep track of the number of task1 and task2 threads
		// using totalThreads.  When totalThreads is equal to 0, all
		// tasks have finished and you can return to the main thread.

		totalThreads++;
	}

#if DEBUG == 1
	printf("Running threads.\n");
#endif
	/* You need to switch from the main thread to the first thread. Use the
	 * global variable currentThread to keep track of the currently
	 * running thread.
	 */
#if MUTEX == MYSEM
	mysem_init(&mutex, 1);
#endif

	// start running your threads here.
	printf("%d is running", currentThread);
	swapcontext(&myMain, &context[0]);
	status[currentThread] = 2;

	/* If you reach this point, your threads have all finished. It is
	 * time to free the stack space created for each thread.
	 */
	for (j = 0; j < THREADS; j++)
	{
		free(myStack[j]);
	}
	printf("==========================\n");
	printf("sharedCounter = %d\n", sharedCounter);
	printf("==========================\n");
#if DEBUG == 1
	printf("Program terminates successfully.\n");
	printf("Note that it is OK for the execution orders\n");
	printf("to differ from one run to the next!\n");
#endif
}

void signalHandler(int signal)
{
	int temp_currentThread = currentThread;
	switch (signal)
	{
	case 100:
		status[0] = 1;
		return;
	case -100:
		status[currentThread] = 0;
	}
	for (int threadIncrementor = 0; threadIncrementor < THREADS; threadIncrementor++)
	{
		if (status[threadIncrementor] == 1 && currentThread != threadIncrementor)
		{
			followUpThread = threadIncrementor;
			currentThread = followUpThread;
			break;
		}
	}
	swapcontext(&context[temp_currentThread], &context[followUpThread]);
	return;
}

void cleanup()
{
	status[followUpThread] = 0;
	totalThreads = totalThreads - 1;
#if MYDEBUG == True
	printf("\n Thread %d just finished \n", followUpThread);
	printf("\n Number of Threads Remaining Are : %d. \n", totalThreads);
#endif
	int threadIncrementor = 0;
#if MYDEBUG == True
	printf("\n Status of the threads are: ");
	printf("\n (0 means finished & 1 means unfinished) \n");
#endif

	while (threadIncrementor < THREADS)
	{

#if MYDEBUG == True
		if (threadIncrementor == (THREADS - 1))
			printf(" Thread %d: %d |\n", threadIncrementor, status[threadIncrementor]);
		else if (threadIncrementor == 0)
			printf("| Thread %d: %d |", threadIncrementor, status[threadIncrementor]);
		else
			printf(" Thread %d: %d |", threadIncrementor, status[threadIncrementor]);
#endif
		threadIncrementor++;
	}
	if (totalThreads == 0)
	{
		swapcontext(&myCleanup, &myMain);
		signalHandler(INTERVAL);
		return;
	}
	else
	{
		signalHandler(INTERVAL);
		return;
	}
}

void task1(int tid)
{
	int i;
	for (i = 0; i < BOUND; i++)
	{
#if MUTEX == POSIX
		sem_wait(&mutex);
		sharedCounter = sharedCounter + 1;
		sem_post(&mutex);
#elif MUTEX == MYSEM
		mysem_wait(&mutex);
		sharedCounter = sharedCounter + 1;
		mysem_post(&mutex);
#elif MUTEX == NONE
		sharedCounter = sharedCounter + 1;
#endif
#if DEBUG == 1
		if (i % PRINT == 0)
			printf("task1 [tid = %d]: sharedCounter = %d.\n", tid, sharedCounter);
#endif
	}
}

void task2(int tid)
{
	int i;
	for (i = 0; i < BOUND; i++)
	{
		//printf("\n\nHELPPPPP U\n\n");
#if MUTEX == POSIX
		sem_wait(&mutex);
		sharedCounter = sharedCounter - 1;
		sem_post(&mutex);
#elif MUTEX == MYSEM
		mysem_wait(&mutex);
		sharedCounter = sharedCounter - 1;
		mysem_post(&mutex);
#elif MUTEX == NONE
		sharedCounter = sharedCounter - 1;
#endif
#if DEBUG == 1
		if (i % PRINT == 0)
			printf("task2 [tid = %d]: sharedCounter = %d.\n", tid, sharedCounter);
#endif
	}
}
