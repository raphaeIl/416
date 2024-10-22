#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"
#include <signal.h>
#include <sys/time.h>
#include <string.h>

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
void simulate_short_task(void* i)
{
	int arg_i = *((int*)i);

	// simulating a fast task (should finish instantly)
	printf("[thread content]: running thread %d...\n", arg_i);
	int a = 0;
	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 1000; j++)
		{
			a += 1;

			/* Test code for yielding, proof that it works: notice that with yielding, 
			   the logs show that this short task ran for 2 time quantums instead of 1 (when time quantum = 1 second)
			if (a == 1234) 
			{
				a = 2000;
				worker_yield();
			}
			*/

			// printf("short task %d: %d\n", arg_i, a);
		}
	}
	
	printf("thread %d result: %d\n", arg_i, a);
}

void simulate_long_task(void* i)
{
	int arg_i = *((int*)i);
	printf("[thread content]: running thread %d...\n", arg_i);
	
	// simulating a lot of work (about 10 seconds)
	int a = 0;
	for (int k = 0; k < 100000; k++)
	{
		for (int j = 0; j < 100000; j++)
		{
			a += 1;

			// Test code for exiting, notice that if we exit at some point, 
			// the long task thread exits immediately and the whole program terminates (with thread2 joined)
			/*
			if (a == 1234) 
			{
				worker_exit(NULL);
			}
			*/
			// printf("long task %d: %d\n", arg_i, a);
		}
	}
	
	printf("thread %d result: %d\n", arg_i, a);
}

int main(int argc, char **argv) {
	/* Implement HERE */
	int *arg1 = malloc(sizeof(int));

	worker_t thread1;
    *arg1 = 1;
	pthread_create(&thread1, NULL, (void*)&simulate_short_task, arg1);

	printf("back to main thread, thread %d created and started\n", thread1);

	int *arg2 = malloc(sizeof(int));
	worker_t thread2;

    *arg2 = 2;
	pthread_create(&thread2, NULL, (void*)&simulate_long_task, arg2);

	printf("back to main thread, thread %d created and started\n", thread2);

	int *arg3 = malloc(sizeof(int));
	worker_t thread3;

    *arg3 = 3;
	pthread_create(&thread3, NULL, (void*)&simulate_long_task, arg3);
	printf("back to main thread, thread %d created and started\n", thread3);
	
	printf("joining threads...\n");
	pthread_join(thread2, NULL);

	printf("main thread done\n");

	print_app_stats();
	fprintf(stderr, "***************************\n");

	// while (1)
	// {
	// }

	return 0;
}
