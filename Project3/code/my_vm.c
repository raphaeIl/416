#include "my_vm.h"

vm_t vm;

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating

    vm.physical_memory = malloc(MEMSIZE);

    if (vm.physical_memory == NULL) 
    {
        printf("Not enough memory available!\n");
        exit(EXIT_FAILURE);
    }

    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
    vm.num_physical_pages = MEMSIZE / PGSIZE;
    vm.num_virtual_pages = MAX_MEMSIZE / PGSIZE;

    vm.physical_bitmap = calloc(vm.num_physical_pages, sizeof(char)); // calloc to set all initial values to 0 (unallocated)
    vm.virtual_bitmap = calloc(vm.num_virtual_pages, sizeof(char)); 

    printf("Physical memory and bitmaps initialized.\n");
    printf("# physical pages: %d, # virtual pages: %d\n", vm.num_physical_pages, vm.num_virtual_pages);

    pthread_mutex_init(&vm.global_lock, NULL);
    TLB_init();
}


/*
 * Part 2: Initialize the core struct for managing tlb
 */
void TLB_init() {
    for (int i = 0; i < TLB_ENTRIES; i++) 
    {
        vm.tlb_store.tlb_entries[i].valid = 0; // Mark all entries as invalid
    }
    
    vm.tlb_store.hits = 0;
    vm.tlb_store.misses = 0;
}

/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 * 
 * Note: Make sure this is thread safe by locking around critical 
 *       data structures touched when interacting with the TLB
 */
int TLB_add(void *va, void *pa)
{
    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    unsigned int vpn = extract_bits(va, 12, 32); 
    unsigned int ppn = extract_bits(pa, 12, 32);
    int index = vpn % TLB_ENTRIES;

    tlb_entry_t new_tlb_entry;

    new_tlb_entry.virtual_page = vpn;
    new_tlb_entry.physical_page = ppn;
    new_tlb_entry.valid = 1;

    vm.tlb_store.tlb_entries[index] = new_tlb_entry;
    return 0;
}

int TLB_remove(void *va) { // this simply just make the entry invalid by setting the valid int
    unsigned int vpn = extract_bits(va, 12, 32); 
    int index = vpn % TLB_ENTRIES;
    
    vm.tlb_store.tlb_entries[index].valid = 0;
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
    unsigned int vpn = extract_bits(va, 12, 32); // vpn: extract the first 20 bits which is the vpn (offset removed)
    int index = vpn % TLB_ENTRIES; // calculating the index this mapping should be stored at (vpn % num_sets)

    if (vm.tlb_store.tlb_entries[index].valid && vm.tlb_store.tlb_entries[index].virtual_page == vpn) 
    {
        vm.tlb_store.hits++;

        unsigned int ppn = vm.tlb_store.tlb_entries[index].physical_page;
        unsigned int found_physical_address = ppn * PGSIZE;

        return (pte_t*)found_physical_address;
    }

    vm.tlb_store.misses++;
    return nullptr;
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void print_TLB_missrate()
{
    /*Part 2 Code here to calculate and print the TLB miss rate*/
    int total = vm.tlb_store.hits + vm.tlb_store.misses;

    double miss_rate = (total == 0) ? 0.0 : ((double)vm.tlb_store.misses / total) * 100; // just incase if we get no lookups which probably wont happen idk but dont wanna cause divide by 0

    printf("Hits: %d, Misses: %d\n", vm.tlb_store.hits, vm.tlb_store.misses);
    printf("TLB miss rate: %.2lf%%\n", miss_rate);
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
    pte_t* pa = TLB_check(va);

    if (pa != nullptr) // cached translation found inside tlb
    {
        // printf("Translation exists in the TLB!\n");
        return pa;
    }

    unsigned int page_dir_index, page_table_index, offset;

    extract_data_from_va(va, &page_dir_index, &page_table_index, &offset);

    pte_t* second_level_table = (pte_t*)pgdir[page_dir_index];

    if (second_level_table == NULL) // page table entries are actual c pointers so we use NULL here instead of nullptr (which is used for our custom pointers)
    {
        printf("That page table entry does not exist!\n");
        return nullptr;
    }
    
    unsigned int target_entry = second_level_table[page_table_index];
    unsigned int physical_address = target_entry << 12 | offset;

    TLB_add(va, (void*)physical_address);

    printf("physical address: 0x%x\n", physical_address);
    return (pte_t *)physical_address;
}


/*
    The function takes a page directory address, virtual address, physical address
    as an argument, and sets a page table entry. This function will walk the page
    directory to see if there is an existing mapping for a virtual address. If the
    virtual address is not present, then a new entry will be added
*/
int map_page(pde_t* pgdir, void *va, void *pa)
{
    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */
    unsigned int page_dir_index, page_table_index, offset;
    unsigned int physical_address = (unsigned int)pa;

    extract_data_from_va(va, &page_dir_index, &page_table_index, &offset);

    if (!pgdir[page_dir_index])
    {
        printf("mapping not found, allowcating second-level table.\n");
        pgdir[page_dir_index] = (unsigned int)calloc(1024, sizeof(pte_t)); // 10 bits 2^10 = 1024

        // set to nullptr (-1) instead of 0
        pte_t* second_level_table = (pte_t *)pgdir[page_dir_index];
        for (int i = 0; i < 1024; i++) 
        {
            second_level_table[i] = (pte_t)nullptr;
        }
    }   

    pte_t* second_level_table = (pte_t*)pgdir[page_dir_index];

    if (second_level_table[page_table_index] != nullptr)
    {
        printf("virtual address: 0x%x with page_table_index: 0x%d already has a mapping! of: 0x%x\n", va, second_level_table[page_table_index], page_table_index);
        return -1;
    }

    unsigned int ppn = extract_bits(physical_address, 12, 32); // extract the top bits of the physical address, this removes the offset since it is not needed to be stored (the va contains the offsets)
    second_level_table[page_table_index] = ppn;

    // printf("map_page: 0x%x=0x%x, : 0x%x\n", page_table_index, ppn, translate(vm.pgdir, va));
    

    if (TLB_check(va) == nullptr) // cached translation found inside tlb
    {
        TLB_add(va, pa);
    }

    return 0;
}


/*
    Function that gets the next available page
    Note that this will find the next available space in the virtual memory
    that has enough continous space for *num_pages*, not just any opened single space. 
*/
void* get_next_avail_virtual(int num_pages) {
    for (int i = 0; i <= vm.num_virtual_pages - num_pages; i++)
    {
        int is_available = 1;

        for (int j = i; j < i + num_pages; j++)
        {
            if (get_bit_at_index(vm.virtual_bitmap, j) == 1)
            {
                is_available = 0;
                break;
            }
        }

        if (is_available)
        {
            unsigned int available_virtual_address = i * PGSIZE;
            // printf("avilable_page: %x\n", avilable_page);
            return (void*)available_virtual_address;
        }
    }
    return nullptr; // custom null because the bitmap starts at 0 so 0 is a valid page (not null)
}

void* get_next_avail_physical() {
    for (int i = 0; i < vm.num_physical_pages; i++)
    {
        if (get_bit_at_index(vm.physical_bitmap, i) == 0)
        {
            unsigned int available_physical_address = i * PGSIZE;

            return (void*)available_physical_address;
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
void *n_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

    pthread_mutex_lock(&vm.global_lock);
    void* ret = __n_malloc_internal(num_bytes);
    pthread_mutex_unlock(&vm.global_lock);

    return ret;
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

    pthread_mutex_lock(&vm.global_lock);
    __n_free_internal(va, size);
    pthread_mutex_unlock(&vm.global_lock);
}

int put_data(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */

    pthread_mutex_lock(&vm.global_lock);
    int ret = __put_data_internal(va, val, size);
    pthread_mutex_unlock(&vm.global_lock);
    /*return -1 if put_data failed and 0 if put is successfull*/

    return ret;
}

/*Given a virtual address, this function copies the contents of the page to val*/
void get_data(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */
    pthread_mutex_lock(&vm.global_lock);
    __get_data_internal(va, val, size);
    pthread_mutex_unlock(&vm.global_lock);
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
/*
    This is a custom utils function that will extract a speific segments of bits from an value using a mask,
    from beginning (inclusive) to end (exclusive), or range: [begin, end)
    From the least sigificant bit to the most, so reversed order.
*/ 
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

void extract_data_from_va(void* va, unsigned int* page_dir_index, unsigned int* page_table_index, unsigned int* offset)
{
    int num_offset_bits = (int)log2(PGSIZE); // in a 4k page size config, 12 bits reserved for the offset
    int num_bit_per_level = (NUM_BIT_ADDRESS_SPACE - num_offset_bits) / NUM_LEVELS; // each level 10 bits, assuming evenly divided bits per level for now

    unsigned int virtual_address = (unsigned int)va; // 22, 32
    *page_dir_index = extract_bits(virtual_address, num_offset_bits + num_bit_per_level, NUM_BIT_ADDRESS_SPACE);
    *page_table_index = extract_bits(virtual_address, num_offset_bits, num_offset_bits + num_bit_per_level);
    *offset = extract_bits(virtual_address, 0, num_offset_bits);

    // printf("virtual_address: 0x%x\n", virtual_address);
    // printf("num_offset_bits: %d\n", num_offset_bits);
    // printf("num_bit_per_level: %d\n", num_bit_per_level);
    // printf("page_dir_index (first level): 0x%x\n", *page_dir_index);
    // printf("page_table_index (second level): 0x%x\n", *page_table_index);
    // printf("offset: 0x%x\n", *offset);
}

void* __n_malloc_internal(unsigned int num_bytes) {
    if (vm.physical_memory == NULL)
    {
        set_physical_mem();
        vm.pgdir = calloc(1024, sizeof(pde_t)); // 2^10, 10 bits for first level
    }

    unsigned int num_pages_needed = (int)ceil((double)num_bytes / PGSIZE);

    pte_t next_available_virtual_address = (pte_t)get_next_avail_virtual(num_pages_needed);

    // printf("malloc bytes[%d] pages needed: %d\n", num_bytes, num_pages_needed);

    if (next_available_virtual_address == nullptr)
    {
        printf("No available virtual memory for allocation\n");
        return nullptr;
    }

    // printf("next_available_virtual_address: 0x%x\n", next_available_virtual_address);
    pte_t current_virtual_address = next_available_virtual_address;

    for (int i = 0; i < num_pages_needed; i++) {

        unsigned int current_physical_address = (unsigned int)get_next_avail_physical(); 
        
        if (current_physical_address == nullptr) 
        {
            printf("Out of physical memory\n");
            return nullptr;
        }

        // Map the virtual page to the physical page
        int map_result = map_page(vm.pgdir, (void*)current_virtual_address, (void*)current_physical_address);
        
        if (map_result != 0) 
        {
            printf("Failed to map page for virtual address\n");
            return nullptr;
        }

        int virtual_page_index = current_virtual_address / PGSIZE;
        int physical_page_index = current_physical_address / PGSIZE;
        // printf("Mapped: virtual_page_index: %d, physical_page_index: %d\n", virtual_page_index, physical_page_index);

        set_bit_at_index(vm.virtual_bitmap, virtual_page_index);
        set_bit_at_index(vm.physical_bitmap, physical_page_index);
        printf("Mapped virtual address: 0x%x to physical address: 0x%x\n", (void *)current_virtual_address, current_physical_address);

        current_virtual_address += PGSIZE;
    }

    printf("Confirmation - 0x%x -> 0x%x\n", next_available_virtual_address, translate(vm.pgdir, next_available_virtual_address));

    return (void*)next_available_virtual_address;
}

void __n_free_internal(void *va, int size) { // freeing the whole page even if size is not a multiple of PGSIZE
    if (va == nullptr || size <= 0)
    {
        printf("Invalid Pointer or Size\n");
        return;
    }

    unsigned int virtual_address = (unsigned int)va;
    int num_pages_freed = (int)ceil((double)size / PGSIZE);
    
    for (int i = 0; i < num_pages_freed; i++) // looping through and freeing each page, 
    {
        unsigned int current_virtual_address = virtual_address + i * PGSIZE; 
        // printf("test: 0x%x\n", i * PGSIZE);
        // printf("test2: 0x%x\n", virtual_address + i * PGSIZE);
        // printf("current_virtual_address: 0x%x\n", current_virtual_address);

        unsigned int current_physical_address = translate(vm.pgdir, (void *)current_virtual_address);

        printf("Freeing memory at: current_virtual_address: 0x%x, physical: 0x%x\n", current_virtual_address, current_physical_address);
        
        if (current_physical_address == nullptr) 
        {
            printf("Virtual address has not been mapped or allocated\n");
            return;
        }

        int virtual_page_index = current_virtual_address / PGSIZE;
        int physical_page_index = current_physical_address / PGSIZE;

        unsigned int page_dir_index, page_table_index, offset;
        extract_data_from_va(current_virtual_address, &page_dir_index, &page_table_index, &offset);

        pte_t* second_level_table = (pte_t*)vm.pgdir[page_dir_index];

        if (second_level_table != NULL) 
        {
            // printf("setting page_table_index[0x%x]to nullptr\n", page_table_index);
            second_level_table[page_table_index] = nullptr;
        }

        clear_bit_at_index(vm.physical_bitmap, physical_page_index); 
        clear_bit_at_index(vm.virtual_bitmap, physical_page_index);
        TLB_remove(current_virtual_address);

        // printf("cleared bit at physical_bitmap index: %d, physical_page_index: %d\n", virtual_page_index, physical_page_index);
    }
}

int __put_data_internal(void *va, void *val, int size) {
    if (va == nullptr || !val || size <= 0) {
        printf("Invalid Pointer or Size\n");
        return -1;
    }

    if (vm.physical_memory == NULL)
    {
        printf("Please malloc first\n");
        return -1;
    }

    unsigned long virtual_address = (unsigned long)va;
    char* data = (char*)val;
    int remaining_data_size = size;

    while (remaining_data_size > 0) {
        unsigned int current_physical_address = translate(vm.pgdir, (void *)virtual_address);

        if (current_physical_address == nullptr) 
        {
            printf("Virtual address has not been mapped or allocated\n");
            return -1;
        }

        unsigned int page_dir_index, page_table_index, offset;
        extract_data_from_va(va, &page_dir_index, &page_table_index, &offset);

        int copy_size = PGSIZE;

        if (copy_size > remaining_data_size) 
        {
            copy_size = remaining_data_size; 
        }

        // printf("copy_size: %d\n", copy_size);
        // printf("offset: %d\n", offset);
        // printf("physical_page: 0x%x\n", physical_page);
        // printf("data: %d\n", data);

        memcpy(vm.physical_memory + current_physical_address, data, copy_size);

        virtual_address += copy_size; // move to the next virtual address
        data += copy_size;    // move to the next source data position
        remaining_data_size -= copy_size; // decrease remaining data size
    }

    return 0;
}

void __get_data_internal(void *va, void *val, int size) {
    if (va == nullptr || !val || size <= 0) {
        printf("Invalid Pointer or Size\n");
        return;
    }

    if (vm.physical_memory == NULL)
    {
        printf("Please malloc first\n");
        return;
    }

    unsigned long virtual_address = (unsigned long)va;
    char* dest = (char*)val;
    int remaining_data_size = size;

    while (remaining_data_size > 0) {

        unsigned int current_physical_address = translate(vm.pgdir, (void *)virtual_address);

        if (current_physical_address == nullptr) 
        {
            printf("Virtual address has not been mapped or allocated\n");
            return -1;
        }

        unsigned int page_dir_index, page_table_index, offset;
        extract_data_from_va(va, &page_dir_index, &page_table_index, &offset);

        int copy_size = PGSIZE;

        if (copy_size > remaining_data_size) 
        {
            copy_size = remaining_data_size; 
        }

        memcpy(dest, vm.physical_memory + current_physical_address, copy_size);

        virtual_address += copy_size; 
        dest += copy_size;  
        remaining_data_size -= copy_size;
    }
}
