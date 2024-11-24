
# all : test
test: ../my_vm.h
	gcc test.c -L../ -lmy_vm -m32 -g -lm -o test
	gcc multi_test.c -L../ -lmy_vm -m32 -g -o mtest -lpthread -lm

ctest:
	gcc ctest.c ../my_vm.c -m32 -lm -g -o ctest

clean:
	rm -rf test mtest ctest

.PHONY: ctest