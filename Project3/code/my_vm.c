#include "my_vm.h"

/*
Function responsible for allocating and setting your physical memory 
*/

unsigned int num_physical_pages;
unsigned int num_virtual_pages;

char* physical_bitmap = NULL; 
char* virtual_bitmap = NULL;   

pde_t *pgdir = NULL;
void* physical_memory = NULL;   

struct tlb tlb_store;

void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating

    physical_memory = malloc(MEMSIZE);

    if (physical_memory == NULL) 
    {
        printf("Not enough memory available");
        exit(EXIT_FAILURE);
    }

    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
    num_physical_pages = MEMSIZE / PGSIZE;
    num_virtual_pages = MAX_MEMSIZE / PGSIZE;

    physical_bitmap = calloc(num_physical_pages, sizeof(int));
    virtual_bitmap = calloc(num_virtual_pages, sizeof(int)); 

    printf("Physical memory and bitmaps initialized.\n");
    printf("# physical pages: %d, # virtual pages: %d\n", num_physical_pages, num_virtual_pages);

    initialize_tlb();
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 * 
 * Note: Make sure this is thread safe by locking around critical 
 *       data structures touched when interacting with the TLB
 */
void initialize_tlb() {
    for (int i = 0; i < TLB_ENTRIES; i++) {
        tlb_store.entries[i].valid = 0; // Mark all entries as invalid
    }
    tlb_store.hits = 0;
    tlb_store.misses = 0;
}

int TLB_add(void *va, void *pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    unsigned long virtual_page = (unsigned long)va / PGSIZE;
    unsigned long physical_page = (unsigned long)pa / PGSIZE;

    // Calculate the TLB index
    int index = virtual_page % TLB_ENTRIES;

    // Add the translation to the TLB
    tlb_store.entries[index].virtual_page = virtual_page;
    tlb_store.entries[index].physical_page = physical_page;
    tlb_store.entries[index].valid = 1; // Mark the entry as valid

    return 0; // Success
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 * 
 * Note: Make sure this is thread safe by locking around critical 
 *       data structures touched when interacting with the TLB
 */
pte_t* TLB_check(void *va) {

    /* Part 2: TLB lookup code here */



   /*This function should return a pte_t pointer*/
    unsigned long virtual_page = (unsigned long)va / PGSIZE;

    // Calculate the TLB index
    int index = virtual_page % TLB_ENTRIES;

    // Check if the entry is valid and matches the virtual page
    if (tlb_store.entries[index].valid &&
        tlb_store.entries[index].virtual_page == virtual_page) {
        tlb_store.hits++; // Record a TLB hit
        return (pte_t *)(tlb_store.entries[index].physical_page * PGSIZE);
    }

    tlb_store.misses++; // Record a TLB miss
    return nullptr; // Translation not found
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/


    int total_lookups = tlb_store.hits + tlb_store.misses;
    miss_rate = (total_lookups == 0) ? 0.0 : ((double)tlb_store.misses / total_lookups) * 100;

    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t* translate(pde_t* pgdir, void* va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */
    // 0001001000 1101000101 011001111000

    pte_t* pa = TLB_check(va);
    if (pa != nullptr) 
    {
        return pa; // Translation found in the TLB
    }

    unsigned int virtual_address = (unsigned int)va;
    unsigned int first_level_address = extract_bits(virtual_address, 22, 32);
    unsigned int second_level_address = extract_bits(virtual_address, 12, 22);
    unsigned int offset = extract_bits(virtual_address, 0, 12);

    // printf("virtual_address: 0x%x\n", virtual_address);
    // printf("first_level_address: 0x%x\n", first_level_address);
    // printf("second_level_address: 0x%x\n", second_level_address);
    // printf("offset: 0x%x\n", offset);

    pte_t* second_level_table = (pte_t*)pgdir[first_level_address];

    if (!second_level_table)
    {
        printf("That page directory does not exist!\n");
        return nullptr;
    }
    
    unsigned int target_entry = second_level_table[second_level_address];
    unsigned int physical_address = target_entry << 12 | offset;

    pte_t *entry = &second_level_table[second_level_address];
    TLB_add(va, (void *)(*entry & ~0xFFF));

    // printf("physical address: 0x%x\n", physical_address);
    return (pte_t *)physical_address;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int map_page(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */
    unsigned int virtual_address = (unsigned int)va;
    unsigned int physical_address = (unsigned int)pa;
    unsigned int first_level_address = extract_bits(virtual_address, 22, 32);
    unsigned int second_level_address = extract_bits(virtual_address, 12, 22);

    // printf("virtual_address: 0x%x\n", virtual_address);
    // printf("first_level_address: 0x%x\n", first_level_address);
    // printf("second_level_address: 0x%x\n", second_level_address);

    if (!pgdir[first_level_address])
    {
        printf("mapping not found, allowcating second-level table.\n");
        pgdir[first_level_address] = (unsigned int)calloc(1024, sizeof(pte_t));

        // set to nullptr (-1) instead of 0
        pte_t *second_level_table = (pte_t *)pgdir[first_level_address];
        for (int i = 0; i < 1024; i++) {
            second_level_table[i] = (pte_t)nullptr;
        }
    }   

    pte_t* second_level_table = (pte_t*)pgdir[first_level_address];
    if (second_level_table[second_level_address] != nullptr)
    {
        printf("That virtual address already has a mapping! of: 0x%x\n", second_level_table[second_level_address]);
        return -1;
    }

    unsigned int physical_offset = extract_bits(physical_address, 12, 32);

    second_level_table[second_level_address] = physical_offset;


    TLB_add(va, pa);
    return 0;
}


/*Function that gets the next available page
*/
void* get_next_avail_virtual(int num_pages) {
    for (int i = 0; i <= num_virtual_pages - num_pages; i++)
    {
        int is_available = 1;

        for (int j = i; j < i + num_pages; j++)
        {
            if (get_bit_at_index(virtual_bitmap, j) == 1)
            {
                is_available = 0;
                break;
            }
        }

        if (is_available)
        {
            printf("is_available!!\n");
            unsigned int avilable_page = i * PGSIZE;
            printf("avilable_page: %x\n", avilable_page);
            return (void*)avilable_page;
        }
    }
    return nullptr; // custom null because the bitmap starts at 0 so 0 is a valid page (not null)
}

void* get_next_avail_physical() {
    for (int i = 0; i < num_physical_pages; i++)
    {
        if (get_bit_at_index(physical_bitmap, i) == 0)
        {
            unsigned int avilable_page = i * PGSIZE;

            return (void*)avilable_page;
        }
    }

    return nullptr; 
}


/* Function responsible for allocating pages and used by the benchmark
 *
 * Note: Make sure this is thread safe by locking around critical 
 *       data structures you plan on updating. Two threads calling
 *       this function at the same time should NOT get the same
 *       or overlapping allocations
*/
void* n_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

    if (physical_memory == NULL)
    {
        set_physical_mem();
        pgdir = calloc(1024, sizeof(pde_t)); // 2^10, 10 bits for first level
    }

    unsigned int num_pages_needed = (num_bytes + PGSIZE - 1) / PGSIZE;
    pte_t next_available_virtual_page_start = (pte_t)get_next_avail_virtual(num_pages_needed);

    printf("current malloc pages needed: %d\n", num_pages_needed);

    if (next_available_virtual_page_start == nullptr)
    {
        printf("No available virtual memory for allocation\n");
        return nullptr;
    }

    printf("next_available_virtual_pages: %x\n", next_available_virtual_page_start);
    pte_t current_virtual_address = next_available_virtual_page_start;

    for (unsigned int i = 0; i < num_pages_needed; i++) {
        // Get the next available physical page
        void* pa = get_next_avail_physical(); // Allocate one physical page
        if (pa == nullptr) {
            fprintf(stderr, "Out of physical memory\n");
            return nullptr;
        }

        // Map the virtual page to the physical page
        int map_result = map_page(pgdir, (void *)(current_virtual_address + i * PGSIZE), pa);
        if (map_result != 0) {
            fprintf(stderr, "Failed to map page for virtual address\n");
            return nullptr;
        }

        unsigned long vpage = (current_virtual_address + i * PGSIZE) / PGSIZE;
        unsigned long ppage = (current_virtual_address + i * PGSIZE) / PGSIZE;
        set_bit_at_index(virtual_bitmap, vpage);
        set_bit_at_index(physical_bitmap, ppage);

        printf("Mapped virtual address: 0x%x to physical address: 0x%x\n", (void *)(current_virtual_address + i * PGSIZE), pa);

    }

    printf("Confirmation - 0x%x -> 0x%x\n", next_available_virtual_page_start, translate(pgdir, next_available_virtual_page_start));

    return next_available_virtual_page_start;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void n_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */

    if (va == nullptr || size <= 0)
    {
        printf("Invalid Pointer or Size\n");
        return;
    }

    unsigned int virtual_address = (unsigned int)va;
    int num_pages_freed = (size + PGSIZE - 1) / PGSIZE;
    
    for (int i = 0; i < num_pages_freed; i++) {
        unsigned long current_virtual_address = virtual_address + i * PGSIZE;
        // printf("test: 0x%x\n", i * PGSIZE);
        // printf("test2: 0x%x\n", virtual_address + i * PGSIZE);
        // printf("current_virtual_address: 0x%x\n", current_virtual_address);
        // Translate the virtual address to a physical address
        pte_t* current_physical_address = translate(pgdir, (void *)current_virtual_address);
        printf("Free: current_virtual_address: 0x%x, physical: 0x%x\n", current_virtual_address, current_physical_address);
        if (current_physical_address == nullptr) {
            printf("Virtual address has not been mapped or allocated\n");
            return;
        }

        // Free the physical page
            
        // unsigned int physical_page = (extract_bits(current_physical_address, 12, 32)) / PGSIZE;
        unsigned int physical_page = ((unsigned int)current_physical_address & ~0xFFF) / PGSIZE; // remove the 12 offset bits
        clear_bit_at_index(physical_bitmap, physical_page); // Mark the physical page as free
        // printf("making physical_page #%d as free\n", physical_page);

        // Remove the page table entry
        unsigned int first_level_address = extract_bits(current_virtual_address, 22, 32);
        unsigned int second_level_address = extract_bits(current_virtual_address, 12, 22);
        pte_t* second_level_table = (pte_t*)pgdir[first_level_address];

        // printf("free i: %d: virtual_address: 0x%x\n", i, current_virtual_address);
        // printf("free i: %d: first_level_address: 0x%x\n", i, first_level_address);
        // printf("free i: %d: second_level_address: 0x%x\n", i, second_level_address);
        // printf("free i: %d: before: 0x%x\n", i, second_level_table[second_level_address]);
        if (second_level_table) {
            // printf("clearing\n");
            second_level_table[second_level_address] = nullptr; // Clear the page table entry
        }

        // printf("free i: %d: after: 0x%x\n", i, second_level_table[second_level_address]);

        // Free the virtual page
        unsigned long virtual_page = current_virtual_address / PGSIZE;
        // printf("making virtual_page #%d as free\n", virtual_page);
        clear_bit_at_index(virtual_bitmap, virtual_page); // Mark the physical page as free
    }
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
 * The function returns 0 if the put is successfull and -1 otherwise.
*/
int put_data(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */

    if (va == nullptr || !val || size <= 0) {
        printf("Invalid Pointer or Size\n");
        return -1;
    }

    if (physical_memory == NULL)
    {
        printf("Please malloc first\n");
        return -1;
    }

    unsigned long virtual_address = (unsigned long)va;
    unsigned char *data = (unsigned char*)val;
    int remaining_data_size = size;

    while (remaining_data_size > 0) {
        // Translate the virtual address to a physical address
        pte_t* current_physical_address = translate(pgdir, (void *)virtual_address);

        if (current_physical_address == nullptr) {
            printf("Virtual address has not been mapped or allocated\n");
            return -1;
        }

        // Get the physical page address
        unsigned long physical_page = (unsigned long)current_physical_address & ~0xFFF; // Mask out the offset bits

        // Calculate the offset within the current page
        int offset = virtual_address & 0xFFF;

        // Calculate how much data can be copied to the current page
        int copy_size = PGSIZE - offset; // Space remaining in the current page
        if (copy_size > remaining_data_size) {
            copy_size = remaining_data_size; // Only copy what remains
        }

        // printf("copy_size: %d\n", copy_size);
        // printf("offset: %d\n", offset);
        // printf("physical_page: 0x%x\n", physical_page);
        // printf("data: %d\n", data);
        // Copy data to the physical memory
        memcpy(physical_memory + (physical_page + offset), data, copy_size);

        // Update pointers and counters
        virtual_address += copy_size; // Move to the next virtual address
        data += copy_size;    // Move to the next source data position
        remaining_data_size -= copy_size; // Decrease remaining data size
    }
    /*return -1 if put_data failed and 0 if put is successfull*/

    return 0;
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_data(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */

    if (va == nullptr || !val || size <= 0) {
        printf("Invalid Pointer or Size\n");
        return;
    }

    if (physical_memory == NULL)
    {
        printf("Please malloc first\n");
        return;
    }

    unsigned long va_int = (unsigned long)va;
    unsigned char *dest = (unsigned char *)val;
    int remaining_size = size;

    while (remaining_size > 0) {
        // Translate the virtual address to a physical address
        pte_t *pa = translate(pgdir, (void *)va_int);
        if (pa == nullptr) {
            fprintf(stderr, "Invalid virtual address: %p\n", (void *)va_int);
            return;
        }

        // Get the physical page address
        unsigned long pa_int = (unsigned long)pa & ~0xFFF; // Mask out the offset bits

        // Calculate the offset within the current page
        int offset = va_int & 0xFFF;

        // Calculate how much data can be read from the current page
        int copy_size = PGSIZE - offset; // Space remaining in the current page
        if (copy_size > remaining_size) {
            copy_size = remaining_size; // Only copy what remains
        }

        // Read data from the physical memory
        memcpy(dest, physical_memory + (pa_int + offset), copy_size);

        // Update pointers and counters
        va_int += copy_size; // Move to the next virtual address
        dest += copy_size;   // Move to the next destination buffer position
        remaining_size -= copy_size; // Decrease remaining data size
    }

}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_data() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */
    int x, y, val_size = sizeof(int);
    int i, j, k;
    for (i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            unsigned int a, b, c = 0;
            for (k = 0; k < size; k++) {
                int address_a = (unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int));
                int address_b = (unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int));
                get_data( (void *)address_a, &a, sizeof(int));
                get_data( (void *)address_b, &b, sizeof(int));
                printf("Values at the index: %d, %d, %d, %d, %d\n", 
                    a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            printf("This is the c: %d, address: %x!\n", c, address_c);
            put_data((void *)address_c, (void *)&c, sizeof(int));
        }
    }
}

// Bit Util Functions from Project 1
// starting_bit_index from 1
unsigned int extract_bits(unsigned int value, int begin, int end)
{
    unsigned int mask = (1 << (end - begin)) - 1;
    return (value >> begin) & mask;
}

unsigned int get_top_bits(unsigned int value,  int num_bits)
{
	int shift = floor(log2(value)) + 1;

    return value >> (shift - num_bits);
}

/* 
 * Function 2: SETTING A BIT AT AN INDEX 
 * Function to set a bit at "index" bitmap
 */
void set_bit_at_index(char *bitmap, int index)
{
    int byteNum = index / 8;  
    int bitNum = index % 8;  

    bitmap[byteNum] |= (1 << bitNum);
}

void clear_bit_at_index(char *bitmap, int index)
{
    int byteNum = index / 8;  
    int bitNum = index % 8;  

    bitmap[byteNum] &= ~(1 << bitNum); 
}

/* 
 * Function 3: GETTING A BIT AT AN INDEX 
 * Function to get a bit at "index"
 */
int get_bit_at_index(char *bitmap, int index)
{
    int byteNum = index / 8; 
    int bitNum = index % 8; 

    return (bitmap[byteNum] >> bitNum) & 1;
}
