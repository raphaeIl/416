#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

#define NUM_THREADS 5
#define INCREMENT_COUNT 10000

// This example tests basic operations of the mutex lock by running x threads that trys to increment one shared variable

// Shared variable that will be modified by multiple threads
int shared_counter = 0;

// Mutex to protect access to the shared variable
worker_mutex_t mutex;

// Function that each thread will run
void *increment_counter(void *arg) {
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        for (int j = 0; j < INCREMENT_COUNT; j++) {
            // Acquire the mutex lock before modifying the shared variable
            pthread_mutex_lock(&mutex);
            
            // Critical section: safely increment the shared counter
            shared_counter++;
            
            // Release the mutex lock after modifying the shared variable
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

int main() {
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // Array to hold thread IDs
    worker_t threads[NUM_THREADS];

    // Create multiple threads that will increment the shared counter
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, increment_counter, NULL) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the final value of the shared counter
    printf("Final value of shared_counter: %d\n", shared_counter);

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    return 0;
}
