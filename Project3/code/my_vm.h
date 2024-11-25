#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need
#define PGSIZE 4096

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024

// How many Levels does the Page Table have?
#define NUM_LEVELS 2

// Address Space Bits
#define NUM_BIT_ADDRESS_SPACE 32

// Represents a page table entry
typedef unsigned long pte_t;

// Represents a page directory entry
typedef unsigned long pde_t;

#define TLB_ENTRIES 512

// This struct represents the entry in the tlb which caches a vpn -> ppn mapping
// the validation int is for if we want to remove the cached translation
typedef struct {
    unsigned long virtual_page;   // vpn
    unsigned long physical_page;  // ppn
    int valid;                   // valid bit: 1 if entry is valid, 0 otherwise
} tlb_entry_t;

// Structure to represents TLB
struct tlb {
    /*
    * Assume your TLB is a direct mapped TLB with number of entries as TLB_ENTRIES
    * Think about the size of each TLB entry that performs virtual to physical
    * address translation.
    */
    tlb_entry_t tlb_entries[TLB_ENTRIES];
    
    int hits;    
    int misses;  
};

// Main Data Struct for the VM, will be initialized as a global variable
typedef struct {
    int num_physical_pages; // total number of physical pages, based on MEMESIZE and PGSIZE
    int num_virtual_pages; // total number of virtual pages

    char* physical_bitmap; 
    char* virtual_bitmap;   

    pde_t* pgdir; // head of the pgdir
    void* physical_memory; // the actual physical memory

    struct tlb tlb_store; // global variable for the tlb
} vm_t;

// Setup functions
void set_physical_mem();

// TLB Functions
void TLB_init();
int TLB_add(void *va, void *pa);
pte_t *TLB_check(void *va);
int TLB_remove(void *va); // extra function I added for removing the mappings when freeing memory, simply sets valid bit to 0

void print_TLB_missrate();

// Page Table Functions
pte_t* translate(pde_t *pgdir, void *va);
int map_page(pde_t *pgdir, void *va, void* pa);

void* get_next_avail_virtual(int num_pages); // this function gets the next available virtual address (must be contiguous)
void* get_next_avail_physical(); // similar to above, this getes the next available physical address (does not have to be contiguous)

// Allocation functions
void *n_malloc(unsigned int num_bytes);
void n_free(void *va, int size);

// Data operations
int put_data(void *va, void *val, int size);
void get_data(void *va, void *val, int size);
void mat_mult(void *mat1, void *mat2, int size, void *answer);

// I made these "internal" functions for functions that needs to be entirely mutex locked, doing this makes it easier since i can just unlock and lock once, instead of doing it multiple times,
// This might be redundant and can be inefficient but in my opinion for the below functions the whole function is the critical section, so i'm not just locking speific parts
// So n_malloc and n_free are bascially just wrappers for my internal functions (which the internal functions shouldnt ever be called from outside, "private")

/* Internal Functions */
void* __n_malloc_internal(unsigned int num_bytes);
void __n_free_internal(void *va, int size);

int __put_data_internal(void *va, void *val, int size);
void __get_data_internal(void *va, void *val, int size);

// Bit Util Functions from Project 1
void set_bit_at_index(char *bitmap, int index);
void clear_bit_at_index(char *bitmap, int index);
int get_bit_at_index(char *bitmap, int index);

unsigned int extract_bits(unsigned int value, int begin, int end); // this function extracts the specified segment of bits from a value

// Other Util Functions
void extract_data_from_va(void* va, unsigned int* page_dir_index, // this function extracts the three different parts of an address (for a two-level page table)
    unsigned int* page_table_index, unsigned int* offset); // example - for a 4k page size: the page directory index (first 10 bits), the page table index (second 10 bits) and the offset (last 12 bits) 

void extract_page_number_from_address(void* address, unsigned int* page_number); // this extracts the page number from an address, in other works, it removes the offset from an address and returns the first part of it which is the page number
#endif
