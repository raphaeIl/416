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
void run(int i)
{
	printf("[thread content]: running thread %d...\n", i);
	
	// simulating a lot of work (this takes like 5 seconds)
	int a = 0;
	for (int i = 0; i < 10000; i++)
	{
		for (int j = 0; j < 50000; j++)
		{
			a += 1;
		}
	}

	printf("result: %d\n", a);
}

int main(int argc, char **argv) {
	/* Implement HERE */
	int *arg = malloc(sizeof(int));

	worker_t thread;
    *arg = 1;
	worker_create(&thread, NULL, (void*)&run, arg);

	printf("back to main thread, thread1 done\n");

	worker_t thread2;
    *arg = 2;
	worker_create(&thread2, NULL, (void*)&run, arg);

	printf("back to main thread, thread2 done\n");

	worker_t thread3;
    *arg = 3;
	worker_create(&thread3, NULL, (void*)&run, arg);
	
	printf("back to main thread, thread3 done\n");
	
	
	printf("continue other work...\n");

	while(1) {}

	return 0;
}
