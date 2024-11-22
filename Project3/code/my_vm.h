#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need

#define PGSIZE 4096

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024

// Represents a page table entry
typedef unsigned long pte_t;

// Represents a page directory entry
typedef unsigned long pde_t;

#define TLB_ENTRIES 512
#define nullptr ((void *)-1)

struct tlb_entry {
    unsigned long virtual_page;  // Virtual page number
    unsigned long physical_page; // Physical page number
    int valid;                   // Valid bit: 1 if entry is valid, 0 otherwise
};


//Structure to represents TLB
struct tlb {
    /*Assume your TLB is a direct mapped TLB with number of entries as TLB_ENTRIES
    * Think about the size of each TLB entry that performs virtual to physical
    * address translation.
    */
    struct tlb_entry entries[TLB_ENTRIES];
    int hits;    // Number of TLB hits
    int misses;  // Number of TLB misses
};


// Setup functions
void set_physical_mem();

// TLB Functions
void initialize_tlb();
int TLB_add(void *va, void *pa);
pte_t *TLB_check(void *va);
void print_TLB_missrate();

// Page Table Functions
pte_t* translate(pde_t *pgdir, void *va);
int map_page(pde_t *pgdir, void *va, void* pa);

void* get_next_avail_virtual(int num_pages);
void* get_next_avail_physical();

// Allocation functions
void *n_malloc(unsigned int num_bytes);
void n_free(void *va, int size);

// Data operations
int put_data(void *va, void *val, int size);
void get_data(void *va, void *val, int size);
void mat_mult(void *mat1, void *mat2, int size, void *answer);

// Bit Util Functions from Project 1
unsigned int extract_bits(unsigned int value, int begin, int end);
void set_bit_at_index(char *bitmap, int index);
void clear_bit_at_index(char *bitmap, int index);
int get_bit_at_index(char *bitmap, int index);
#endif
