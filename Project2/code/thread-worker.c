// File:	thread-worker.c

// List all group member's name:
// username of iLab:
// iLab Server:
#include "thread-worker.h"

typedef void *(*thread_function)(void*);

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
int thread_id_counter = 0;

scheduler_t scheduler;

thread_function t_func;

void thread_function_wrapper(void* arg)
{
	t_func(arg);

	
	printf("Finished executing, going back to scheduler context\n");
	scheduler.current_thread->tcb->status = READY_STATUS;
	setcontext(scheduler.scheduler_context);
}

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void* arg) {

	// - create Thread Control Block (TCB)
	// - create and initialize the context of this worker thread
	// - allocate space of stack for this thread to run
	// after everything is set, push this thread into run queue and 
	// - make it ready for the execution.

	// YOUR CODE HERE
	if (thread_id_counter == 0) // this means it is the first ever call to worker_create
	{
		sch_init(&scheduler);
	}

	tcb* new_tcb = malloc(sizeof(tcb));

	new_tcb->threadId = thread_id_counter++;
	new_tcb->priority = DEFAULT_PRIO;
	new_tcb->status = WAITING_STATUS;

	ucontext_t* context = malloc(sizeof(ucontext_t));

	if (getcontext(context) < 0){
		perror("getcontext");
		exit(1);
	}

    // Dynamically allocate a stack
	new_tcb->stack = malloc(STACK_SIZE);

	context->uc_link=NULL;
	context->uc_stack.ss_sp=new_tcb->stack;
	context->uc_stack.ss_size=STACK_SIZE;
	context->uc_stack.ss_flags=0;
	new_tcb->context = context;
	thread->tcb = new_tcb;

	t_func = function;

	makecontext(new_tcb->context, (void*)thread_function_wrapper, 1, *((int*)arg));
	printf("Created new worker thread with id: %d\n", new_tcb->threadId);

	sch_schedule(&scheduler, thread);

	// q_printqueue(&scheduler.run_queue.queues[DEFAULT_PRIO]);

    return 0;
};


#ifdef MLFQ
/* This function gets called only for MLFQ scheduling set the worker priority. */
int worker_setschedprio(worker_t thread, int prio) {


   // Set the priority value to your thread's TCB
   // YOUR CODE HERE

   return 0;	

}
#endif



/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	scheduler.current_thread->tcb->status = READY_STATUS;
	swapcontext(scheduler.current_thread->tcb->context, scheduler.scheduler_context);

	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
	
	while (thread.tcb->status == RUNNING_STATUS)
	{

	}

	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	return 0;
};

void timer_schedule_handler(int signum)
{
	printf("Timer expired, going back to scheduler context\n");

	if (scheduler.current_thread->tcb->status != READY_STATUS)
	{
		printf("Thread has not completed it's work yet, readding it to the run queue for the next quantum\n");

		q_enqueue(scheduler.run_queue->queues[DEFAULT_PRIO], scheduler.current_thread);
		q_printqueue(scheduler.run_queue->queues[DEFAULT_PRIO]);
	}

    swapcontext(scheduler.current_thread->tcb->context, scheduler.scheduler_context);  // Swap back to the scheduler context
}

/* scheduler */
static void schedule(scheduler_t* scheduler) {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE
	while (!q_is_empty(scheduler->run_queue->queues[DEFAULT_PRIO]))
	{
		printf("[schedule] start\n");
		worker_t* target_thread = malloc(sizeof(worker_t));

		// determine which thread to run here based on psjf/mlfq
		// ... (hardcoded for now)
		q_dequeue(scheduler->run_queue->queues[DEFAULT_PRIO], target_thread);
		target_thread->tcb->status = RUNNING_STATUS;

		scheduler->current_thread = target_thread;

		printf("switching to thread %d context...\n", scheduler->current_thread->tcb->threadId);
		
		swapcontext(scheduler->scheduler_context, scheduler->current_thread->tcb->context);
		
		printf("switched back to scheduler context\n");

		swapcontext(scheduler->scheduler_context, scheduler->main_context);
		printf("now going back to main\n");
	}

// - schedule policy
#ifndef MLFQ
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need
void ll_init(linkedlist_t* ll)
{
	for (int i = 0; i < NUM_QUEUES; i++) {
        ll->queues[i] = malloc(sizeof(queue_t));
		q_init(ll->queues[i]);
	}
}

void q_init(queue_t* q)
{
    q->front = 0;
    q->back = 0;
}

// YOUR CODE HERE
void q_enqueue(queue_t* q, worker_t* item)
{
	printf("q_enqueue\n");
    q->items[q->back] = item;
	q->back++;
}

void q_dequeue(queue_t* q, worker_t* result)
{
	printf("q_dequeue\n");
	*result = *q->items[q->front];
	q->front++;
}

void q_peek(queue_t* q, worker_t* result)
{
    *result = *q->items[q->front];
}

int q_is_empty(queue_t* q)
{
	return q->front == q->back;
}

void q_printqueue(queue_t* q)
{
	printf("Queue: front: %d, back: %d\n", q->front, q->back);
    for (int i = q->front; i != q->back; i++)
    {
		tcb* curr = q->items[i]->tcb;

        printf("%d - threadId: %d, status: %d\n", i, curr->threadId, curr->status);
    }
}

void sch_init(scheduler_t* scheduler)
{
	scheduler->run_queue = malloc(sizeof(linkedlist_t));
	ll_init(scheduler->run_queue);

    scheduler->scheduler_context = malloc(sizeof(ucontext_t));
	getcontext(scheduler->scheduler_context);

    scheduler->main_stack = malloc(STACK_SIZE);

    scheduler->scheduler_context->uc_link = NULL;
    scheduler->scheduler_context->uc_stack.ss_sp = scheduler->main_stack;
    scheduler->scheduler_context->uc_stack.ss_size = STACK_SIZE;
    scheduler->scheduler_context->uc_stack.ss_flags = 0;

    makecontext(scheduler->scheduler_context, (void (*)(void))schedule, 1, scheduler);

	// set a timer that will run for TIME_QUANTUM which swaps back when expired 
	create_timer(TIME_QUANTUM, 1, timer_schedule_handler);
}

void sch_schedule(scheduler_t* scheduler, worker_t* thread)
{
	q_enqueue(scheduler->run_queue->queues[DEFAULT_PRIO], thread);

    scheduler->main_context = malloc(sizeof(ucontext_t));

	swapcontext(scheduler->main_context, scheduler->scheduler_context);
}

void create_timer(time_t duration, int repeat, __sighandler_t on_expire_handler)
{
	struct sigaction sa;
    struct itimerval timer;

    sa.sa_handler = on_expire_handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = duration;
    timer.it_value.tv_usec = 0;
    
    timer.it_interval.tv_sec = repeat ? duration : 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);
}