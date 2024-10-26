// File:	worker_t.h

// List all group member's name: Michael Liu
// username of iLab: msl196@ilab1.cs.rutgers.edu
// iLab Server: ilab1
#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

typedef uint worker_t;
typedef struct queue_t queue_t;

typedef struct TCB {
	/* add important states in a thread control block */
	int threadId; // thread Id
	int status; // thread status - running, waiting, blocked, ready
	ucontext_t* context; // thread context
	void* stack; // thread stack
	int priority; // thread priority

	void* return_value; // will store the thread's return value if it exists

	/* for PSJF */
	int quantums_elapsed; // this keeps tracks of how many quantums that this thread has ALREADY ran, 
	// assumption: "the more time quantum a thread has run, the longer this job will run to finish"
	
	/* statistics */
	int current_context_switches;
	struct timespec time_enqueued; // this is the time that the thread was first put into the runqueue
	struct timespec time_scheduled; // the time this thread was first scheduled
	struct timespec time_finished; // the time that this thread finished
	// so  turnaround_time = time_finished - time_enqueued
	// and response_time = time_enqueued - time_scheduled
	
	// And more ...
	// YOUR CODE HERE
} tcb; 

/* mutex struct definition */
typedef struct worker_mutex_t {
	/* add something here */
	int locked;
	worker_t owner;

	queue_t* blocked_threads;
// YOUR CODE HERE
} worker_mutex_t;

/* Priority definitions */
#define NUMPRIO 4

#define HIGH_PRIO 3
#define MEDIUM_PRIO 2
#define DEFAULT_PRIO 1
#define LOW_PRIO 0

/* Priority definitions */
#define WAITING_STATUS 0
#define READY_STATUS 1
#define RUNNING_STATUS 2
#define BLOCKED_STATUS 3
#define FINISHED_STATUS 4

#define STACK_SIZE SIGSTKSZ

#define MAX_THREADS 1000

#define TIME_QUANTUM 10 // (in ms)
#define REFRESH_QUANTUM 3 // (this is defined in multiples of TIME_QUANTUM and will trigger every (x * TIME_QUANTUM)ms)

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE
typedef struct node {
	tcb* data;
	struct node* next;
} node_t;

struct queue_t {
	node_t* head;
	node_t* tail;
	int size;
};

typedef struct {
	queue_t* queues[NUMPRIO];
} runqueue_t;

typedef struct {
    runqueue_t* run_queue;

    ucontext_t* main_context; // main context idk where to store this
    ucontext_t* scheduler_context; // scheduler context
	void* scheduler_stack; // scheduler context stack

	tcb* main_thread;
	tcb* current_thread; // currently running thread;

	tcb* thread_table[MAX_THREADS]; // this is a mapping that is used to find threads by their id, kinda waste space probably need better management
} scheduler_t;

/*
	a wrapper like this is necessary since i don't think we can just pass our original
	thread function like `simulate_long_task` directly into makecontext because at the end
	of the function we need to swap the context back so i'm adding a wrapper to do that: thread_function_wrapper
	
	there is probably a better solutinon to this lol
 */ 
typedef struct {
	void *(*original_function)(void*); // original worker function (for example: simulate_long_task)
	int* original_args;  // original function's args
} worker_wrapper_t;

/* Function Declarations: */

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);

/* scheduler */
static void schedule(scheduler_t* scheduler);

static tcb* sched_psjf(scheduler_t* scheduler);

static tcb* sched_mlfq(scheduler_t* scheduler);

static tcb* sched_matrix(scheduler_t* scheduler);

/* Run Queue Operations */
void rq_init(runqueue_t* runqueue);

int rq_is_all_empty(runqueue_t* runqueue); // checks if ALL queues are empty

int rq_get_shortest_runtime(runqueue_t* runqueue, int priority);

int rq_get_index_highest_nonempty(runqueue_t* runqueue); // get index of the highest priority non empty queue in the list of queues

void rq_printlist(runqueue_t* runqueue);

/* Queue Operations */
void q_init(queue_t* q);

void q_enqueue(queue_t* q, tcb* item);

tcb* q_dequeue(queue_t* q);

tcb* q_dequeue_shortest_runtime(queue_t* q);

tcb* q_dequeue_longest_runtime(queue_t* q);

int q_is_empty(queue_t* q);

void q_printqueue(queue_t* q);

void q_destroy(queue_t* q);

/* scheduler_t Operations */
void sch_init(scheduler_t* scheduler);

void sch_schedule(scheduler_t* scheduler, tcb* thread);

/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

/* Util Functions */
void create_timer(time_t duration, int repeat, __sighandler_t on_expire_handler);

void timer_disable();

void calc_and_update_stats();

double get_duration_micro(struct timespec start, struct timespec end);

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#define pthread_setschedprio worker_setschedprio
#endif

#endif
