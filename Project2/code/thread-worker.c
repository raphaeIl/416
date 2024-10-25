// File:	thread-worker.c

// List all group member's name:
// username of iLab:
// iLab Server:
#include "thread-worker.h"

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches = 0;
double avg_turn_time = 0;
double avg_resp_time = 0;

// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
int thread_id_counter = 0;
int time_quantum_counter = 0;

scheduler_t scheduler;

void thread_function_wrapper(void* arg)
{
	worker_wrapper_t* wrapper_data = (worker_wrapper_t*)arg;

	// call the original worker function
	scheduler.current_thread->return_value = wrapper_data->original_function(wrapper_data->original_args);

	scheduler.current_thread->status = FINISHED_STATUS;

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

	if (thread_id_counter >= MAX_THREADS - 1)
	{
		printf("Maximum number of threads allowed reached, please adjust MAX_THREADS");
		return -1;
	}

	tcb* new_tcb = malloc(sizeof(tcb));

	new_tcb->threadId = thread_id_counter++;
	new_tcb->priority = DEFAULT_PRIO;
	new_tcb->status = WAITING_STATUS;
	new_tcb->return_value = NULL;

	new_tcb->time_enqueued.tv_nsec = 0;
	new_tcb->time_enqueued.tv_sec = 0;
	new_tcb->time_scheduled.tv_nsec = 0;
	new_tcb->time_scheduled.tv_sec = 0;
	new_tcb->time_finished.tv_nsec = 0;
	new_tcb->time_finished.tv_sec = 0;

	new_tcb->quantums_elapsed = 0;

	ucontext_t* context = malloc(sizeof(ucontext_t));

	if (getcontext(context) < 0){
		perror("getcontext");
		exit(1);
	}

    // Dynamically allocate a stack
	new_tcb->stack = malloc(STACK_SIZE);

	context->uc_link = scheduler.scheduler_context;
	context->uc_stack.ss_sp=new_tcb->stack;
	context->uc_stack.ss_size=STACK_SIZE;
	context->uc_stack.ss_flags=0;
	new_tcb->context = context;
	*thread = new_tcb->threadId;

	worker_wrapper_t *wrapper_data = malloc(sizeof(worker_wrapper_t));

	wrapper_data->original_function = function;
	wrapper_data->original_args = arg;

	makecontext(new_tcb->context, (void*)thread_function_wrapper, 1, wrapper_data);

	scheduler.thread_table[new_tcb->threadId] = new_tcb;
	sch_schedule(&scheduler, new_tcb);
    return 0;
};


#ifdef MLFQ
/* This function gets called only for MLFQ scheduling set the worker priority. */
int worker_setschedprio(worker_t thread, int prio) {

   // Set the priority value to your thread's TCB
   // YOUR CODE HERE
	scheduler.thread_table[thread]->priority = prio;
	
   return 0;	
}
#endif



/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE

	if (scheduler.current_thread == NULL || scheduler.current_thread->status == BLOCKED_STATUS)
	{
		// printf("no thread is running, cannot yield\n");
		return 1;
	}

	scheduler.current_thread->status = READY_STATUS;

	// printf("yielding thread id: %d\n", scheduler.current_thread->threadId);
	swapcontext(scheduler.current_thread->context, scheduler.scheduler_context);

	return 0;
};

/* terminate a thread */  // ptr that is saved
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	// YOUR CODE HERE,.

	scheduler.current_thread->status = FINISHED_STATUS;

	if (value_ptr != NULL)
	{
		scheduler.current_thread->return_value = value_ptr;
	}

	swapcontext(scheduler.current_thread->context, scheduler.scheduler_context);

};


// need tcb* in struct tcb for threads that the current threads are waiting, curr=t3->join(t1): tcb(t3)->joined_thread = t1, meaning that t3 will need to wait for t1 and t3 will be blocked
/* Wait for thread termination */     // ptr that needs to be set with the worker_exit() return value of the thread
int worker_join(worker_t thread, void **value_ptr) {

	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
	// YOUR CODE HERE

	tcb* target_thread = scheduler.thread_table[thread]; // find thread by id

	while (target_thread->status != FINISHED_STATUS)
	{
		worker_yield();
	}

	if (value_ptr != NULL)
	{
		*value_ptr = target_thread->return_value;
	}


	// free(target_thread->context);
	// free(target_thread->stack);
	// free(target_thread);
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	mutex->locked = 0;
	mutex->owner = -1;
	mutex->blocked_threads = malloc(sizeof(queue_t));

	q_init(mutex->blocked_threads);
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {
	
	// - use the built-in test-and-set atomic function to test the mutex
	// - if the mutex is acquired successfully, enter the critical section
	// - if acquiring mutex fails, push current thread into block list and
	// context switch to the scheduler thread

	// YOUR CODE HERE
	if (__sync_lock_test_and_set(&mutex->locked, 1)) // this will keep running until lock becomes available
	{
		scheduler.current_thread->status = BLOCKED_STATUS;
		q_enqueue(mutex->blocked_threads, scheduler.current_thread);
		worker_yield();
		return 1;
	}

	// mutex acquired, enter critical section
	mutex->owner = scheduler.current_thread->threadId;
	return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	__sync_lock_release(&mutex->locked);
	mutex->owner = -1;

	while (!q_is_empty(mutex->blocked_threads))
	{
		sch_schedule(&scheduler, q_dequeue(mutex->blocked_threads));
	}

	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init
	q_destroy(mutex->blocked_threads);

	return 0;
};
// turn this off when updating scheduler_t or anything, no looping
void timer_schedule_handler(int signum)
{
	// this moves the lowest prio to highest periodically to prevent starvation, which is only for MLFQ
	#ifdef MLFQ
		time_quantum_counter++;

		if (time_quantum_counter % REFRESH_QUANTUM == 0) // refresh time reached, move LOW_PRIO threads to HIGH_PRIO
		{
			queue_t* low_prio_q = scheduler.run_queue->queues[LOW_PRIO];

			while (!q_is_empty(low_prio_q))
			{	
				tcb* target_thread = q_dequeue(low_prio_q); 
				target_thread->priority = HIGH_PRIO;
				q_enqueue(scheduler.run_queue->queues[HIGH_PRIO], target_thread);
			}

			// rq_printlist(scheduler.run_queue);
		}
	#endif	

	if (scheduler.current_thread == NULL)
	{
	    swapcontext(scheduler.main_context, scheduler.scheduler_context);  // Swap back to the scheduler context
	}
	else
	{
	    swapcontext(scheduler.current_thread->context, scheduler.scheduler_context);  // Swap back to the scheduler context
	}
	

}

/* scheduler */
static void schedule(scheduler_t* scheduler) {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// YOUR CODE HERE
	while (1)
	{
		// determine which thread to run here based on psjf/mlfq
		// ... (hardcoded for now)

		if (rq_is_all_empty(scheduler->run_queue))
		{
			continue; // no threads scheduled, so skip
		}

		// rq_printlist(scheduler->run_queue); // this prints out the entire queue in a nice format

		tcb* target_thread = NULL;
		
		#ifdef MLFQ
			target_thread = sched_mlfq(scheduler);
		#else
			target_thread = sched_psjf(scheduler);
		#endif

		if (target_thread == NULL)
		{
			printf("Something went wrong when dequeuing a thread.");
			continue;
		}

		scheduler->current_thread = target_thread;
		scheduler->current_thread->status = RUNNING_STATUS;
		
		if (scheduler->current_thread->time_scheduled.tv_nsec == 0 && scheduler->current_thread->time_scheduled.tv_sec == 0) // this is when the current thread is being scheduled and ran for the first time
		{
			clock_gettime(CLOCK_REALTIME, &scheduler->current_thread->time_scheduled);
		}

		scheduler->current_thread->current_context_switches++;
		swapcontext(scheduler->scheduler_context, scheduler->current_thread->context);

		scheduler->current_thread->quantums_elapsed++;

		if (scheduler->current_thread == NULL)
		{
			continue;
		}
		
		if (scheduler->current_thread->status == RUNNING_STATUS || scheduler->current_thread->status == READY_STATUS)
		{
			scheduler->current_thread->status = READY_STATUS;

			// MLFQ: move the thread to the next lower runqueue priority
			// PSJF: only using DEFAULT_PRIO
			#ifdef MLFQ
				int index_lower_q = scheduler->current_thread->priority - 1 > LOW_PRIO ? scheduler->current_thread->priority - 1 : LOW_PRIO;
				scheduler->current_thread->priority = index_lower_q;
				q_enqueue(scheduler->run_queue->queues[index_lower_q], scheduler->current_thread);
			#else
				q_enqueue(scheduler->run_queue->queues[DEFAULT_PRIO], scheduler->current_thread);
			#endif	

		}

		else if (scheduler->current_thread->status == BLOCKED_STATUS)
		{
			scheduler->current_thread = NULL;
		}

		else if (scheduler->current_thread->status == FINISHED_STATUS)
		{
			clock_gettime(CLOCK_REALTIME, &scheduler->current_thread->time_finished);

			calc_and_update_stats();

			// free(target_thread->context);
			// free(target_thread->stack);
			// free(target_thread);
			scheduler->current_thread = NULL;
		}
	}

// - schedule policy
#ifndef MLFQ
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static tcb* sched_psjf(scheduler_t* scheduler) {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)
	// YOUR CODE HERE

	// dequeue thread with the shortest runtime (assumed with elapsed quantum)
	return q_dequeue_shortest_runtime(scheduler->run_queue->queues[DEFAULT_PRIO]); // since psjf will not be using multi level queue, I will only use the DEFAULT_PRIO queue and all others will be useless
}


/* Preemptive MLFQ scheduling algorithm */
static tcb* sched_mlfq(scheduler_t* scheduler) {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE

	return q_dequeue(scheduler->run_queue->queues[rq_get_index_highest_nonempty(scheduler->run_queue)]);
}

void calc_and_update_stats()
{
	int tol_context_switches = 0;
	double tol_turn_time = 0;
	double tol_resp_time = 0;

	for (int i = 1; i < thread_id_counter - 1; i++)
	{
		tcb* curr = scheduler.thread_table[i];

		tol_context_switches += curr->current_context_switches;
		tol_turn_time += get_duration_micro(curr->time_enqueued, curr->time_finished);
		tol_resp_time += get_duration_micro(curr->time_enqueued, curr->time_scheduled);
	}
	
	tot_cntx_switches = tol_context_switches;
	avg_turn_time = tol_turn_time / (thread_id_counter + 1);
	avg_resp_time = tol_resp_time / (thread_id_counter + 1);
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}

double get_duration_micro(struct timespec start, struct timespec end)
{
	double duration = (double)((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000);

	if (duration < 0)
	{
		duration *= -1;
	}

	return duration;
}

// Feel free to add any other functions you need
void rq_init(runqueue_t* runqueue)
{
	for (int i = 0; i < NUMPRIO; i++) {
        runqueue->queues[i] = malloc(sizeof(queue_t));
		q_init(runqueue->queues[i]);
	}
}

int rq_is_all_empty(runqueue_t* runqueue)
{
	for (int i = 0; i < NUMPRIO; i++)
	{
		if (!q_is_empty(runqueue->queues[i]))
		{
			return 0;
		}

	}

	return 1;
}

int rq_get_index_highest_nonempty(runqueue_t* runqueue)
{
	for (int i = NUMPRIO - 1; i >= 0; i--)
	{
		if (!q_is_empty(runqueue->queues[i]))
		{
			return i;
		}
	}

	return -1;
}

void rq_printlist(runqueue_t* runqueue)
{
	timer_disable();

	printf("\n-------------Run Queue Status-------------\n");
	printf("High Priority:\n");
	q_printqueue(runqueue->queues[HIGH_PRIO]);
	printf("\n");

	printf("Medium Priority:\n");
	q_printqueue(runqueue->queues[MEDIUM_PRIO]);
	printf("\n");

	printf("Default Priority:\n");
	q_printqueue(runqueue->queues[DEFAULT_PRIO]);
	printf("\n");

	printf("Low Priority:\n");
	q_printqueue(runqueue->queues[LOW_PRIO]);
	printf("------------------------------------------\n");

	create_timer(TIME_QUANTUM, 1, timer_schedule_handler);
}

void q_init(queue_t* q)
{
    q->head = NULL;
    q->tail = NULL;
	q->size = 0;
}

// YOUR CODE HERE
void q_enqueue(queue_t* q, tcb* item)
{
	timer_disable();

	node_t* new_node = (node_t*)malloc(sizeof(node_t));
	
	new_node->data = item;
	new_node->next = NULL;

	if (q->head == NULL)
	{
		q->head = new_node;
		q->tail = new_node;
	}
	else 
	{
		q->tail->next = new_node;
		q->tail = new_node;
	}

	q->size++;

	create_timer(TIME_QUANTUM, 1, timer_schedule_handler);
}

tcb* q_dequeue(queue_t* q)
{
	timer_disable();

    node_t* result = q->head;

	q->head = q->head->next;
	q->size--;

	create_timer(TIME_QUANTUM, 1, timer_schedule_handler);

	tcb* result_data = result->data;

	free(result);
    return result_data;
}

tcb* q_dequeue_shortest_runtime(queue_t* q)
{
	timer_disable();

	node_t* result = q->head;
    node_t* curr = q->head;
	node_t* prev = NULL;
	node_t* result_prev = result->next;

	while (curr != NULL)
	{
		if (curr->data->quantums_elapsed < result->data->quantums_elapsed)
		{
			result = curr;
        	result_prev = prev;
		}

		prev = curr;
		curr = curr->next;
	}

	if (result->data->threadId == q->head->data->threadId)
	{
		q->head = result->next;
	}
	else
	{
		result_prev->next = result->next;
	}

	create_timer(TIME_QUANTUM, 1, timer_schedule_handler);
	return result->data;
}

int q_is_empty(queue_t* q)
{
	return q->head == NULL;
}

void q_printqueue(queue_t* q)
{
	timer_disable();

	// printf("Queue: front: %d, back: %d\n", q->front, q->back);
	node_t* curr = q->head;

	while (curr != NULL)
	{
		printf("\t");

		if (curr->data->threadId == 0) // main thread id always 0
			printf("MainThread");
		else
			printf("Thread");

        printf("[id=%d, status=%d, priority=%d]\n", curr->data->threadId, curr->data->status, curr->data->priority);

		curr = curr->next;
	}

	create_timer(TIME_QUANTUM, 1, timer_schedule_handler);
}

void q_destroy(queue_t* q)
{
	free(q);
}

void sch_init(scheduler_t* scheduler)
{
	scheduler->run_queue = malloc(sizeof(runqueue_t));
	rq_init(scheduler->run_queue);

    scheduler->main_context = malloc(sizeof(ucontext_t));

	getcontext(scheduler->main_context);

	scheduler->main_thread = malloc(sizeof(tcb));

	scheduler->main_thread->threadId = thread_id_counter++;

	if (scheduler->main_thread->threadId != 0)
	{
		printf("Something went wrong during scheduler initialization, main thread id is not 0\n"); // need this cause later we use index 0 to find main thread
		exit(-1);
	}

	scheduler->main_thread->priority = DEFAULT_PRIO;
	scheduler->main_thread->status = READY_STATUS;
	scheduler->main_thread->context = scheduler->main_context;

	printf("Created main thread with id: %d\n", scheduler->main_thread->threadId);
	
	sch_schedule(scheduler, scheduler->main_thread);

    scheduler->scheduler_context = malloc(sizeof(ucontext_t));
	getcontext(scheduler->scheduler_context);

    scheduler->scheduler_stack = malloc(STACK_SIZE);
    scheduler->scheduler_context->uc_link = NULL;
    scheduler->scheduler_context->uc_stack.ss_sp = scheduler->scheduler_stack;
    scheduler->scheduler_context->uc_stack.ss_size = STACK_SIZE;
    scheduler->scheduler_context->uc_stack.ss_flags = 0;

    makecontext(scheduler->scheduler_context, (void (*)(void))schedule, 1, scheduler);

	scheduler->current_thread = NULL;

	// set a timer that will run for TIME_QUANTUM which swaps back when expired 
	create_timer(TIME_QUANTUM, 1, timer_schedule_handler);
}

void sch_schedule(scheduler_t* scheduler, tcb* thread)
{
	thread->status = READY_STATUS;

	// sorry for the misleading function name but this is actually when the thread was just created and put into the run queue
	clock_gettime(CLOCK_REALTIME, &thread->time_enqueued);

	int target_priority = -1;
	
	// only the multi level queue if MLFQ, otherwise default to DEFAULT_PRIO and ignore all other queues
	#ifdef MLFQ
		target_priority = thread->priority;
	#else
		target_priority = DEFAULT_PRIO;
	#endif

	q_enqueue(scheduler->run_queue->queues[target_priority], thread);
}

void create_timer(suseconds_t duration_ms, int repeat, __sighandler_t on_expire_handler)
{
	struct sigaction sa;
    struct itimerval timer;

    sa.sa_handler = on_expire_handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = duration_ms * 1000;
    
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = repeat ? duration_ms * 1000 : 0;

	// printf("timer created\n");
    setitimer(ITIMER_REAL, &timer, NULL);
}

void timer_disable() {
    struct itimerval timer;

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0; 
    timer.it_interval.tv_usec = 0;

	// printf("timer disabled\n");

	setitimer(ITIMER_REAL, &timer, NULL);
}