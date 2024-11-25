all : test

test: ../my_vm.h
	gcc test.c -L../ -lmy_vm -m32 -lm -o test
	gcc multi_test.c -L../ -lmy_vm -m32 -lm -o mtest -lpthread

frag: 
	gcc test.c ../my_vm_frag.c -m32 -lm -g -o test_frag
	gcc multi_test.c ../my_vm_frag.c -m32 -lm -g -o mtest_frag -lpthread

m64: 
	gcc test.c ../my_vm64.c -m64 -lm -g -o test_m64
	gcc multi_test.c ../my_vm64.c -m64 -lm -g -o mtest_m64 -lpthread

clean:
	rm -rf test mtest