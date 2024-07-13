
#include "mems.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    mems_init();
    int* ptr[10];

    /*
    This allocates 10 arrays
    */
    printf("\n------- Allocated virtual addresses [mems_malloc] -------\n");

    // Testing mems_malloc and mems_free
    int *ptr1 = (int *)mems_malloc(sizeof(int) * 10);
    if (ptr1 != NULL) {
        for (int i = 0; i < 10; i++) {
            ptr1[i] = i + 1;
            printf("Virtual address: %p\n ", (void *)&ptr1[i]);
        }
        printf("\n");

        mems_free(ptr1);
    }

    // Testing mems_malloc for a larger memory size
    char *ptr2 = (char *)mems_malloc(sizeof(char) * 1000);
    if (ptr2 != NULL) {
        strcpy(ptr2, "This is a test string.");
        printf("%s\n", ptr2);
        mems_free(ptr2);
    }
   
    /*
    In this section we are tring to write value to 1st index of array[0] (here it is 0 based indexing).
    We get get value of both the 0th index and 1st index of array[0] by using function mems_get.
    Then we write value to 1st index using 1st index pointer and try to access it via 0th index pointer.

    This section is show that even if we have allocated an array using mems_malloc but we can 
    retrive MeMS physical address of any of the element from that array using mems_get. 
    */
    printf("\n------ Assigning value to Virtual address [mems_get] -----\n");

    // Testing mems_get
    void *ptr3 = mems_malloc(sizeof(double));
    if (ptr3 != NULL) {
        double *d_ptr = (double *)ptr3;
        *d_ptr = 3.14;
        void *phys_addr = mems_get(ptr3);
        //printf("Physical address corresponding to the virtual address %p is %p\n", ptr3, phys_addr);
        printf("Virtual address: %p\tPhysical Address: %p\n", ptr3, phys_addr);
        printf("Value written: %f\n", *d_ptr); // print the address of index 1 

        mems_free(ptr3);
    }

        /*
    This shows the stats of the MeMS system.  
    */
    printf("\n--------- Printing Stats [mems_print_stats] --------\n");
    // Testing mems_print_stats
    mems_print_stats();

    /*
    This section shows the effect of freeing up space on free list and also the effect of 
    reallocating the space that will be fullfilled by the free list.
    */
    printf("\n--------- Freeing up the memory [mems_free] --------\n");
    mems_free(ptr2);
    mems_print_stats();

    printf("\n--------- Reallocating memory [mems_malloc] --------\n");
    ptr2 = (char *)mems_malloc(sizeof(char) * 500);
    mems_print_stats();
    //ptr[3] = (int*)mems_malloc(sizeof(int)*250);
    //mems_print_stats();
    
    printf("\n--------- Unmapping all memory [mems_finish] --------\n\n");
    mems_finish();
    return 0;
}
