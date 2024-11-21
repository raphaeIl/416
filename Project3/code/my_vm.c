#include "my_vm.h"

/*
Function responsible for allocating and setting your physical memory 
*/

unsigned int num_physical_pages;
unsigned int num_virtual_pages;

char* physical_bitmap = NULL; 
char* virtual_bitmap = NULL;   

void* physical_memory = NULL;   

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

    return -1;
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

   return NULL;
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
    // unsigned long va_int = (unsigned long)va;
    unsigned int virtual_address = (unsigned int)va;
    unsigned int first_level_address = extract_bits(virtual_address, 22, 32);
    unsigned int second_level_address = extract_bits(virtual_address, 12, 22);
    unsigned int offset = extract_bits(virtual_address, 0, 12);

    printf("virtual_address: 0x%x\n", virtual_address);
    printf("first_level_address: 0x%x\n", first_level_address);
    printf("second_level_address: 0x%x\n", second_level_address);
    printf("offset: 0x%x\n", offset);

    pde_t* second_level_table = (pde_t*)pgdir[first_level_address];

    if (!second_level_table)
    {
        printf("That page directory does not exist!\n");
        return -1;
    }
    
    unsigned int target_entry = second_level_table[second_level_address];
    unsigned int physical_address = target_entry << 12 | offset;

    printf("physical address: 0x%x\n", physical_address);
    return (pte_t *)physical_address;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
map_page(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 
    for (unsigned int i = 0; i < num_pages / 8; i++) 
    {
        if (physical_bitmap[i] != 0xFF) 
        { 
            for (unsigned int j = 0; j < 8; j++)
            {
                if (!(physical_bitmap[i] & (1 << j))) 
                { 
                    

                    return (i * 8) + j; 
                }
            }
        }
    }
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

    unsigned int num_pages_needed = (num_bytes + PGSIZE - 1) / PGSIZE;

    unsigned int free_page = 0;
    for (free_page = 0; free_page < num_virtual_pages; free_page++) 
    {
        if (!(physical_bitmap[free_page / 8] & (1 << (free_page % 8)))) 
        {
            break;
        }
    }

    if (free_page + num_pages_needed > num_virtual_pages) 
    {
        return NULL;
    }

    for (unsigned int i = 0; i < num_pages_needed; i++) 
    {
        physical_bitmap[(free_page + i) / 8] |= (1 << ((free_page + i) % 8));
    }

    void *virtual_address = (void *)(free_page * PGSIZE);

    return virtual_address;
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

    unsigned int num_pages_to_free = (size + PGSIZE - 1) / PGSIZE;
    unsigned int start_page = (unsigned int)va / PGSIZE;

    for (unsigned int i = 0; i < num_pages_to_free; i++) {
        unsigned int page = start_page + i;
        
        if (!(physical_bitmap[page / 8] & (1 << (page % 8)))) 
        {
            return; 
        }

        physical_bitmap[page / 8] &= ~(1 << (page % 8));
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


    /*return -1 if put_data failed and 0 if put is successfull*/

    return -1;
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_data(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */

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
