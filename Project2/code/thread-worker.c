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

	printf("going back to scheduler context\n");
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

	tcb new_tcb;

	new_tcb.threadId = thread_id_counter++;
	new_tcb.priority = DEFAULT_PRIO;
	new_tcb.status = WAITING_STATUS;

	ucontext_t context;

	if (getcontext(&context) < 0){
		perror("getcontext");
		exit(1);
	}

    // Dynamically allocate a stack
	new_tcb.stack = malloc(STACK_SIZE);

	context.uc_link=NULL;
	context.uc_stack.ss_sp=new_tcb.stack;
	context.uc_stack.ss_size=STACK_SIZE;
	context.uc_stack.ss_flags=0;
	new_tcb.context = &context;
	thread->tcb = &new_tcb;

	t_func = function;

	makecontext(new_tcb.context, (void*)thread_function_wrapper, 1, *((int*)arg));
	printf("Created new worker thread with id: %d\n", new_tcb.threadId);

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

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

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
void q_init(queue_t* q)
{
    q->front = 0;
    q->back = 0;
}

// YOUR CODE HERE
void q_enqueue(queue_t* q, worker_t* item)
{
    q->items[q->back++] = item;
}

void q_dequeue(queue_t* q, worker_t* result)
{
	*result = *q->items[q->front++];
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
    for (int i = q->front; i < q->back; i++)
    {
		tcb* curr = q->items[i]->tcb;

        printf("%d - threadId: %d, status: %d\n", i, curr->threadId, curr->status);
    }
}

void sch_init(scheduler_t* scheduler)
{
    scheduler->scheduler_context = malloc(sizeof(ucontext_t));
	getcontext(scheduler->scheduler_context);

    scheduler->main_stack = malloc(STACK_SIZE);

    scheduler->scheduler_context->uc_link = NULL;
    scheduler->scheduler_context->uc_stack.ss_sp = scheduler->main_stack;
    scheduler->scheduler_context->uc_stack.ss_size = STACK_SIZE;
    scheduler->scheduler_context->uc_stack.ss_flags = 0;

    makecontext(scheduler->scheduler_context, (void (*)(void))sch_switch, 1, scheduler);
}

void sch_switch(scheduler_t* scheduler)
{
	while (1)
	{
		worker_t target_thread;

		// determine which thread to run here based on psjf/mlfq
		// ... (hardcoded for now)
		q_dequeue(&scheduler->run_queue.queues[DEFAULT_PRIO], &target_thread);

		printf("switching to thread %d context...\n", target_thread.tcb->threadId);
		swapcontext(scheduler->scheduler_context, target_thread.tcb->context);
		
		printf("switched back to scheduler context\n");

		swapcontext(scheduler->scheduler_context, scheduler->main_context);
		printf("now going back to main\n");
	}


}

void sch_schedule(scheduler_t* scheduler, worker_t* thread)
{
	q_enqueue(&scheduler->run_queue.queues[thread->tcb->priority], thread);

	printf("swapcontext\n");
    scheduler->main_context = malloc(sizeof(ucontext_t));

	swapcontext(scheduler->main_context, scheduler->scheduler_context);
}