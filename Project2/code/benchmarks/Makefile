# Default to empty if SCHED is not set
SCHED ?= MLFQ

# Use SCHED_FLAG only if SCHED is set
SCHED_FLAG := $(if $(SCHED),-D$(SCHED),)

CC = gcc
CFLAGS = -g -w $(SCHED_FLAG)


all:: clean parallel_cal vector_multiply external_cal test

parallel_cal:
	$(CC) $(CFLAGS) -pthread -o parallel_cal ../thread-worker.c parallel_cal.c

vector_multiply:
	$(CC) $(CFLAGS) -pthread -o vector_multiply ../thread-worker.c vector_multiply.c

external_cal:
	$(CC) $(CFLAGS) -pthread -o external_cal ../thread-worker.c external_cal.c 

test:
	$(CC) $(CFLAGS) test.c ../thread-worker.c -g -o test

clean:
	# rm -rf testcase test parallel_cal vector_multiply external_cal *.o ./record/ *.dSYM
	rm -rf testcase test parallel_cal vector_multiply external_cal *.o *.dSYM
