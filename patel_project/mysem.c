#define _XOPEN_SOURCE 600

#include "mysem.h"

#define DEBUG 0

#define WAITSIGNAL -100

#define POSTSIGNAL 100

#define THREADSCONST 16

extern int currentThread;
int key;

void atomic_swap(volatile long long int *lock)
{
	/* This function can be used to atomically 
 	 * swap a value in %r14 to variable lock.
 	 */

	__asm__ __volatile__(
		"LOCK: xorq %r14, %r14;"
		"xchg %r14,(%rdi);"
		"andq %r14,%r14;"
		"jz LOCK;");

	/* if you want to assign a value from a 
	 * register to a variable, you can see the example below. 
	 * register int tmp asm("%r14");
	 */
}

int mysem_init(mysem_t *mysem, int val)
{
	int i;
	if (mysem != NULL)
	{
		mysem->val = val;
		mysem->lock = 1;
		mysem->blkThreads = 0;
		for (i = 0; i < THREADS; i++)
		{
			mysem->Q[i] = -1;
		}
		return 1;
	}
	else
		return 0;
}

void mysem_wait(mysem_t *mysem)
{
	if (mysem == NULL)
	{
#if DEBUG == 1
		printf("Blocking Thread %d (val = %d)\n", currentThread, mysem->val);
#endif
		assert(mysem->val >= 0);
	}
	else if (mysem != NULL)
	{
		atomic_swap(&mysem->lock);
		while (1)
		{
			if (mysem->val == 0)
			{
				mysem->blkThreads++;
				mysem->Q[currentThread] = THREADSCONST;
				mysem->lock = 1;
				signalHandler(WAITSIGNAL);
				atomic_swap(&mysem->lock);
			}
			else
			{
				break;
			}
		}
		mysem->val--;
		mysem->lock = 1;
#if DEBUG == 1
		printf("Blocking Thread %d (val = %d)\n", currentThread, mysem->val);
#endif
		assert(mysem->val >= 0);
	}
}

void mysem_post(mysem_t *mysem)
{
	atomic_swap(&mysem->lock);
	mysem->val = mysem->val + 1;
	int threadCounter;

	int check = 1;
	while (threadCounter < THREADS)
	{
		if (mysem->Q[threadCounter] == THREADSCONST)
		{
			mysem->blkThreads--;
			mysem->Q[threadCounter] = -1;
			mysem->lock = 1;
			key = threadCounter;
			signalHandler(POSTSIGNAL);
			check = 1;
			threadCounter++;
			break;
		}
		else
		{
			threadCounter++;
		}
	}
	if (check == 0)
	{
		assert(mysem->val <= 1);
		mysem->lock = 1;
#if DEBUG == 1
		printf("Wake up thread %d.\n", tid);
#endif
		return;
	}
	else
	{
#if DEBUG == 1
		printf("Wake up thread %d.\n", tid);
#endif
		return;
	}
}

int mysem_value(mysem_t *mysem)
{
	/* This function simply returns the semaphore value. */
	return mysem->val;
}
