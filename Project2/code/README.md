# Project 2
User-level Thread Library and Scheduler

### Contributors:
Michael Liu msl196

## Build
#### Compile and Link all files
```
make
```

## Scheduler Modes

### PSJF


### MLFQ:


## General Structure
### `TCB` - This is the thread control block that represents our threads and contains all data for thread
- `ThreadId` - This is the thread's unique identifier, notice the **main** thread will always have the id of `0`
- `status` - The status of the thread, this can take the value of:
    - `0` - `WAITING_STATUS`: The thread is waiting to be initalized
    - `1` - `READY_STATUS`: All thread data has been initalized and ready to be started
    - `2` - `RUNNING_STATUS`: The thread is currently executing
    - `3` - `BLOCKED_STATUS`: The thread has been blocked, it will not continue execution until it has been unblocked
    - `4` - `FINISHED_STATUS`: The thread has finished executing
- `context` - context of this thread
- `stack` - pointer to the stack of the context
- `priority` - the priority of the thread, this will only be useful in MLFQ
- `return_value` - this is a pointer to return value which will be returned in worker_join when a thread finishes.

    Below variables that keeps track of statistics
- `current_context_switches`: This is used to keep track of the total number of context switches
- `time_enqueued`: this is the time that the thread was first put into the runqueue
- `time_scheduled`: the time this thread was first scheduled
- `time_finished`: the time that this thread finished
    
    All these timesare used to calculate `turnaround_time` and `response_time`

### `scheduler_t` - This is the main struct that will contain most of the data needed for the scheduler, a global instance of this is initalized at the start.
- `run_queue`: This is my scheduler's runqueue, the structure of this is explained below
- `main_context`: The main context
- `scheduler_context`: The scheduler context
- `scheduler_stack`: Pointer to the scheduler context stack
- `main_thread`: This is a pointer to the main thread, stored here for easy acces
- `current_thread`: This is the current thread that is being executed/running
- `thread_table`: this is a mapping that is used to find threads by their id, kinda waste space probably need better management



## Project Description 
In this project, we implemented a pthread-like library that contains two type of schedulers, PSJF and MLFQ
### Notes: 
NOTE: 