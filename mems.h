#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>

#define PAGE_SIZE 4096

typedef struct Node {
    void* address;
    size_t size;
    int isHole;
    struct Node* next;
    struct Node* prev;
} Node;

#define MAX_ADDRESS_MAPS 1000 // Adjust the maximum number of address mappings as needed

typedef struct AddressMap {
    void* virtual_address;
    void* physical_address;
    struct AddressMap* next;
    struct AddressMap* prev;
} AddressMap;

static Node head = {0};
static void *starting_address = NULL;

static AddressMap* address_map_head = NULL;
static AddressMap* address_map_tail = NULL;
static AddressMap address_map_pool[MAX_ADDRESS_MAPS];
static int address_map_index = 0;

void mems_init() {
    head.address = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1,0);    
        if (head.address == MAP_FAILED) {
        perror("Error in initializing MeMS");
        exit(EXIT_FAILURE);
    }
    head.size = PAGE_SIZE;
    head.isHole = 1;
    head.next = NULL;
    head.prev = NULL;
    starting_address = 0;
}

void mems_finish() {
    Node *current = head.next;
    Node *temp;
    while (current != NULL) {
        temp = current;
        current = current->next;
        if (munmap(temp->address, temp->size) == -1) {
            perror("Error in finishing MeMS");
            exit(EXIT_FAILURE);
        }
    }
    if (munmap(head.address, head.size) == -1) {
        perror("Error in finishing MeMS");
        exit(EXIT_FAILURE);
    }
}

void *mems_malloc(size_t size) {

    size_t rounded_size = (size + PAGE_SIZE -1) / PAGE_SIZE * PAGE_SIZE;
    Node *current = &head;
    while (current != NULL) {
        if (current->isHole && current->size >= rounded_size) {
            if (current->size > rounded_size) {
                Node *newNode = (Node *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1,0); 
                if (newNode == MAP_FAILED) {
                   perror("Allocation Failure");
                   return NULL;
                }
                newNode->address = current->address + rounded_size;
                newNode->size = current->size - rounded_size;
                newNode->isHole = 1;
                newNode->next = current->next;
                newNode->prev = current;
                if (current->next != NULL) {
                    current->next->prev = newNode;
                }
                current->next = newNode;
                current->size = rounded_size;
            }
            current->isHole = 0;
            return current->address;
        }
        current = current->next;
    }
    void *ptr = mmap(NULL, rounded_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1,0);
    if (ptr == MAP_FAILED) {
        perror("Error in mems_malloc");
        return NULL;
    }
    Node *newNode = (Node *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1,0);
    if (newNode == MAP_FAILED) {
        perror("Allocation Failure");
        munmap(ptr, rounded_size);
        return NULL;
    }
    newNode->address = ptr;
    newNode->size = rounded_size;
    newNode->isHole = 0;
    newNode->next = head.next;
    newNode->prev = &head;
    if (head.next != NULL) {
        head.next->prev = newNode;
    }
    head.next = newNode;
    return newNode->address;
}

//size_t allocate_memory(size_t size) {
	//size_t remaining_size = size;
		//while (remaining_size > 0) {
      			//void *allocated_address = mems_malloc_internal(PAGE_SIZE);
      			//if (allocated_address == NULL) {
         			//perror("Error in mems_malloc");
         			//return NULL;
      			//}
      			//remaining_size -= PAGE_SIZE;
		//}

		//return ptr;
//}

void mems_print_stats() {
    Node *current = &head;
    int total_pages = 0;
    size_t unused_memory = 0;

    printf("-----MeMS SYSTEM STATS-----\n");

    while (current != NULL) {
        if (current->isHole) {
            unused_memory += current->size;
        } else {
            total_pages += (current->size + PAGE_SIZE - 1) / PAGE_SIZE;
        }

        printf("MAIN[%p:%p]-> ", current->address, (char *)current->address + current->size - 1);
        if (current->isHole) {
            printf("H[%p:%p] <-> ", current->address, (char *)current->address + current->size -1);
        } else {
            printf("P[%p:%p] <-> ", current->address, (char *)current->address + current->size - 1);
        }
        current = current->next;
    }
    printf("NULL\n");

    printf("Pages used:\n%d\n", total_pages);
    printf("Space unused: %zu\n", unused_memory);

    int main_chain_length = 0;
    current = &head;
    while (current != NULL) {
        main_chain_length++;
        current = current->next;
    }
    printf("Main Chain Length:\n%d\n", main_chain_length);
    size_t *sub_chain_length = (size_t *)malloc(main_chain_length * sizeof(size_t));
    if (sub_chain_length == NULL) {
       fprintf(stderr, "Memory allocation failed for sub_chain_length\n");
       exit(EXIT_FAILURE);
    }

    size_t i = 0;
    for (current = &head; current != NULL; current = current->next) {
        if (current->isHole) {
           sub_chain_length[i]++;
        }else{
          i++;
        }
    }
    //for (int i = 0; i < main_chain_length; i++) {
        //sub_chain_length[i] = 0;
    //}

    //current = &head;
    //int i = 0;
    //while (current != NULL) {
        //if (current->isHole) {
            //sub_chain_length[i]++;
        //} else {
            //i++;
        //}
        //current = current->next;
    //}

    printf("Sub-chain Length array: [");
    for (size_t j = 0; j < main_chain_length - 1; j++) {
        printf("%zu, ", sub_chain_length[j]);
    }
    printf("%zu]\n", sub_chain_length[main_chain_length - 1]);

    free(sub_chain_length);
}

void *mems_get(void *v_ptr) {
    if (v_ptr == NULL) {
        printf("Invalid virtual address provided\n");
        return NULL;
    }

    Node* current = &head;
    while (current != NULL) {
        if (current->address == v_ptr) {
            return current->address;
        }
        current = current->next;
    }

    printf("Virtual address not found in MeMS system\n");
    return NULL;
}

void mems_free(void *v_ptr) {
    Node *current = &head;
    while (current != NULL) {
        if (current->address == v_ptr) {
            current->isHole = 1;
            // Merge adjacent holes if possible
            if (current->prev != NULL && current->prev->isHole) {
                current->prev->size += current->size;
                current->prev->next = current->next;
                if (current->next != NULL) {
                    current->next->prev = current->prev;
                }
                current = current->prev;
            }
            if (current->next != NULL && current->next->isHole) {
                current->size += current->next->size;
                current->next = current->next->next;
                if (current->next != NULL) {
                    current->next->prev = current;
                }
            }
            break;
        }
        current = current->next;
    }
}

