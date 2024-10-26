#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../thread-worker.h"

#define SIZE 3000 // size of the matrix
#define NUM_THREADS 40 // 40 threads 

/* Implement your code for extra-credit here */
// this generate a matrix of size * size
void generateMatrix(int matrix[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) 
    {
        for (int j = 0; j < SIZE; j++) 
        {
            matrix[i][j] = (int)(rand() % 1000); // make it so that the each number is not that big
        }
    }

    return matrix;
}


int mat1[SIZE][SIZE];
int mat2[SIZE][SIZE];
int product[SIZE][SIZE];

// this is the function that each thread will run, where args is the thread id, 
// this will be used to calculate the region of the matrix that this thread is responsible for multiplying since we're splitting up the work among the threads 
void *multiply(void *arg) { 
    int thread_id = *(int *)arg;
    int i, j, k;
    int rows_per_thread = SIZE / NUM_THREADS;
    int start_row = thread_id * rows_per_thread;
    int end_row = (thread_id + 1) * rows_per_thread;

    for (i = start_row; i < end_row; i++) {
        for (j = 0; j < SIZE; j++) {
            product[i][j] = 0;
            for (k = 0; k < SIZE; k++) {
                product[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }

}

int main(int argc, char **argv) {

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    generateMatrix(mat1);
    generateMatrix(mat2);

    struct timespec start, end;
    
    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < NUM_THREADS; i++) 
    {
        thread_ids[i] = i;

        pthread_create(&threads[i], NULL, multiply, &thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) 
    {
        pthread_join(threads[i], NULL);
    }

    fprintf(stderr, "***************************\n");

    clock_gettime(CLOCK_REALTIME, &end);

    printf("Total run time: %lu micro-seconds\n",
            (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000);


#ifdef USE_WORKERS
        print_app_stats();
        fprintf(stderr, "***************************\n");
#endif

    return 0;
}

