
#include "bsp/sys/buffer.h"

// Test code 

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h> 


#define MAX_DATA_LEN 16
#define BUFFER_SIZE (MAX_DATA_LEN/2)
#define ITERATION_COUNT 5

#define TEST_QUEUE 1

#ifdef TEST_QUEUE
#define PUSH_ARRAY_FUNC Buffer_Queue_PushArray_uint8_t
#define POP_ARRAY_FUNC  Buffer_Queue_PopArray_uint8_t
#define GETSIZE_FUNC    Buffer_Queue_GetSize
#endif

#ifdef TEST_STACK
#define PUSH_ARRAY_FUNC Buffer_Stack_PushArray_uint8_t
#define POP_ARRAY_FUNC  Buffer_Stack_PopArray_uint8_t
#define GETSIZE_FUNC    Buffer_Stack_GetSize
#endif

void reverseArray(int *arr, int size) {
    int *reverseArr = (int *) malloc(size * sizeof(int));
    int i, j;

    // Reverse the array
    for (i = size - 1, j = 0; i >= 0; i--, j++) {
        reverseArr[j] = arr[i];
    }

    // Copy the reversed array back to the original array
    for (i = 0; i < size; i++) {
        arr[i] = reverseArr[i];
    }

    // Free dynamically allocated memory
    free(reverseArr);
}


int main() {
    srand(time(NULL)); // Seed the random number generator

    Buffer_uint8_t queue;
    uint8_t data[MAX_DATA_LEN];

    // Initialize the queue
    uint8_t buffer[BUFFER_SIZE]; // Example buffer with 256 bytes of storage
    queue.buf_ptr = buffer;
    queue.len = sizeof(buffer);
    queue.r_ptr = queue.buf_ptr;
    queue.w_ptr = queue.buf_ptr;

    int num_iterations = ITERATION_COUNT; // Example number of iterations
    uint8_t* all_data_pushed = (uint8_t*) malloc(sizeof(uint8_t)*MAX_DATA_LEN*num_iterations);
    uint8_t* all_data_popped = (uint8_t*) malloc(sizeof(uint8_t)*MAX_DATA_LEN*num_iterations);
    int total_push_cnt = 0;
    int total_pop_cnt = 0;

    for (int i = 0; i < num_iterations; i++) 
    {
        // First push and pop sequence
        printf("Push and pop sequence:\n");
        int push_len1 = rand() % MAX_DATA_LEN + 1; // Random length of data to push
        for (int j = 0; j < push_len1; j++) {
            data[j] = rand() % 256; // Example data range from 0 to 255
        }
        int actual_push_cnt = PUSH_ARRAY_FUNC(&queue, data, push_len1);
        printf("Try pushing %d , actual pushed %d bytes: \t",push_len1, actual_push_cnt);
        for (int j = 0; j < actual_push_cnt; j++) {
            all_data_pushed[total_push_cnt++] = data[j];
            printf("%02x ", data[j]);
        }
        printf("\n");
        int pop_len1 = rand() % push_len1 + 1; // Random length of data to pop
        int actual_pop_cnt =POP_ARRAY_FUNC(&queue, data, pop_len1);
        printf("Try popping %d ,actual pop %d bytes: \t",pop_len1, actual_pop_cnt);
        for (int j = 0; j < actual_pop_cnt; j++) {
            all_data_popped[total_pop_cnt++] = data[j];
            printf("%02x ", data[j]);
        }
        printf("\nRemaning %ld\n",GETSIZE_FUNC(&queue));
    }
        printf("\n");


    int pop_len_all = GETSIZE_FUNC(&queue);//PopAll
    printf("Popping all %d bytes: ", pop_len_all);
    POP_ARRAY_FUNC(&queue, data, pop_len_all);
    for (int j = 0; j < pop_len_all; j++) 
    {
        all_data_popped[total_pop_cnt++] = data[j];
        printf("%02x ", data[j]);
    }

    bool error=false;
    if(total_push_cnt!=total_pop_cnt)
    {
        printf("Error: Data pushed len and popped len are not equal\n");
        error=true;
    }
    else
    {

        for (int i = 0; i < total_push_cnt; i++) 
        {
            if (all_data_pushed[i] != all_data_popped[i])
            {
                error=true;
                printf("Error: Data pushed and popped are not equal\n");
                break;
            }
        }
    }
    
    printf("\n");
    if (error)
        printf("Finish with error.\n");
    else
        printf("Finish all ok.\n");
    free(all_data_pushed);
    free(all_data_popped);

    return 0;
}
