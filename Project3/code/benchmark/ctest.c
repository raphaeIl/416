#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../my_vm.h"

void test_translate() {
    // Create mock page directory and page tables
    pde_t* pgdir = (pde_t*)malloc(1024 * sizeof(pde_t)); // First-level page directory (1024 entries)
    pte_t* level2_table = (pte_t*)malloc(1024 * sizeof(pte_t)); // Second-level page table (1024 entries)
    pte_t* level2_table2 = (pte_t*)malloc(1024 * sizeof(pte_t)); // Second-level page table (1024 entries)

    // Mock setup
    unsigned int physical_page = 0xABCDE000; // Mock physical page frame
    // unsigned int virtual_addr = 0x12345678; // Example virtual address
    unsigned int virtual_addr = 0x13345678; // Example virtual address

    // Populate the page tables
    pgdir[0x48] = level2_table; // First-level entry (valid)
    pgdir[0x4C] = level2_table2; // First-level entry (valid)
    level2_table[0x345] = 0x988;
    level2_table2[0x345] = 0x112;
    // Translate the virtual address
    pte_t* physical_addr = translate(pgdir, (void*)(virtual_addr));

    // Print results
    printf("Virtual address: 0x%x\n", (unsigned int)virtual_addr);
    printf("Physical address: 0x%x\n", (unsigned long)physical_addr);
}


int main() {

    test_translate();
}