
# Project 3
User-level Memory Management

### Contributors:
Michael Liu msl196

## Note
This was ran on kill.cs.rutgers.edu

## Build
The default part 1 and part 2 implementations are in the original files `my_vm.h` and `my_vm.c`. Simply run `make` in ./code to build them and make inside ./benchmark to buld and run the tests

Also please re-run the make commands and try again if the program just doesn't run or breaks.

### Extra Credit - 4-level Page Table
The implementation for the 4-level page table with 64-bit address space is inside `my_vm64.h` and `my_vm64.c`, to run the tests, simply go inside ./benchmark and run
```
make m64
```

### Extra Credit - Reducing Fragmentation
The implementation for reducing fragmentation is inside `my_vm_frag.h` and `my_vm_frag.c`, to run the tests, simply go inside ./benchmark and run
```
make frag
```