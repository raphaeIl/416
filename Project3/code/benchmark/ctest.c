#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../my_vm.h"

void test_get_next_avail() {
    set_physical_mem();
    // Request 2 pages
    void* va = get_next_avail_virtual(2);
    if (va != nullptr) {
        printf("Found available block at virtual address: 0x%x\n", va);
    } else {
        printf("No available block found\n");
    }

    // Request 4 pages (should skip over allocated pages 4-6)
    va = get_next_avail_virtual(4);
    if (va != nullptr) {
        printf("Found available block at virtual address: 0x%x\n", va);
    } else {
        printf("No available block found\n");
    }
}

void test_map() {
     pde_t *pgdir = calloc(1024, sizeof(pde_t));
    if (!pgdir) {
        perror("Failed to allocate page directory");
        exit(1);
    }

    // Virtual and physical addresses
    void *va = (void *)0x13345678;
    void *pa = (void *)0xA2C678;

    // Map the page
    int result = map_page(pgdir, va, pa);
    if (result == 0) {
        printf("Mapping successful: %p -> %p\n", va, pa);
    } else {
        fprintf(stderr, "Mapping failed\n");
    }

    // Verify the mapping using translate
    pte_t *translated = translate(pgdir, va);
    printf("Translation: %p -> %p\n", va, translated);
}

void test_malloc() {
    void *a = n_malloc(400);
    int old_a = (int)a;
    void *b = n_malloc(4097);
    void *c = n_malloc(400);

    printf("allocated a: 0x%x\n", a);
    printf("allocated b: 0x%x\n", b);
    printf("allocated c: 0x%x\n", c);

    n_free(b, 4097);
    printf("freed b: 0x%x\n", b);

    void* d = n_malloc(10000);
    printf("allocated d: 0x%x\n", d);

    void* e = n_malloc(1);
    printf("allocated e: 0x%x\n", e);

    void* f = n_malloc(4096);
    printf("allocated e: 0x%x\n", f);


    // int data = 342;

    // // Write the data to memory
    // if (put_data(d, &data, sizeof(data)) == 0) {
    //     printf("Successfully wrote %ld bytes of data to virtual address 0x%x\n", sizeof(data), d);
    // } else {
    //     printf("Failed to write data to virtual address 0x%x\n", d);
    // }

    // // // Verify the data
    // char read_data[8192];
    // get_data(d, read_data, sizeof(read_data));

    // printf("read_data: %d\n", *((int*)read_data));

}

void test_n_free() {
    // Allocate 2 pages
    void *va = n_malloc(2 * PGSIZE);

    printf("Allocated 2 pages starting at 0x%x\n", va);

    // Free the allocated pages
    n_free(va, 2 * PGSIZE);
    printf("Freed\n");
    void *va2 = n_malloc(2 * PGSIZE);

    printf("Allocate after free virtual address: 0x%x\n", va2);

}

void test_put_data() {
    // Allocate 2 pages of memory
    void *va = n_malloc(2 * PGSIZE);
    if (va == nullptr) {
        printf("Failed to allocate memory\n");
        return;
    }
    printf("Allocated 2 pages starting at 0x%x\n", va);

    // Data to write
    char data[8192];
    for (int i = 0; i < sizeof(data); i++) {
        data[i] = i % 256; // Fill with sample data
    }

    // Write the data to memory
    if (put_data(va, data, sizeof(data)) == 0) {
        printf("Successfully wrote %ld bytes of data to virtual address 0x%x\n", sizeof(data), va);
    } else {
        printf("Failed to write data to virtual address 0x%x\n", va);
    }

    // // Verify the data
    char read_data[8192];
    get_data(va, read_data, sizeof(read_data));
    if (memcmp(data, read_data, sizeof(data)) == 0) {
        printf("Data verification successful\n");
    } else {
        printf("Data verification failed\n");
    }
}

void test_get_data() {
    // Allocate 2 pages of memory
    void *va = n_malloc(2 * PGSIZE);
    if (va == nullptr) {
        printf("Failed to allocate memory\n");
        return;
    }
    printf("Allocated 2 pages starting at %p\n", va);

    // Data to write
    char write_data[8192];
    for (int i = 0; i < sizeof(write_data); i++) {
        write_data[i] = i % 256; // Fill with sample data
    }

    // Write the data to memory
    if (put_data(va, write_data, sizeof(write_data)) == 0) {
        printf("Successfully wrote %ld bytes of data to virtual address %p\n", sizeof(write_data), va);
    } else {
        printf("Failed to write data to virtual address %p\n", va);
    }

    // Read the data back
    char read_data[8192];
    get_data(va, read_data, sizeof(read_data));

    // Verify the data
    if (memcmp(write_data, read_data, sizeof(write_data)) == 0) {
        printf("Data verification successful\n");
    } else {
        printf("Data verification failed\n");
    }
}

void test_translate() {
    // Create mock page directory and page tables
    pde_t* pgdir = (pde_t*)malloc(1024 * sizeof(pde_t)); // First-level page directory (1024 entries)
    pte_t* level2_table = (pte_t*)malloc(1024 * sizeof(pte_t)); // Second-level page table (1024 entries)
    pte_t* level2_table2 = (pte_t*)malloc(1024 * sizeof(pte_t)); // Second-level page table (1024 entries)

    // Mock setup
    // unsigned int physical_page = 0xABCDE000; // Mock physical page frame
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
    printf("Physical address: 0x%x\n", (unsigned int)physical_addr);
}

int main() {
    // test_map();
    // test_translate();
    // test_get_next_avail();
    // test_n_free();

    // test_put_data();
    // test_get_data();


    test_malloc();
    print_TLB_missrate();
}