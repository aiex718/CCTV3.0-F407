#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "string.h"

#include "bsp/sys/mem_utils.c"

#define DATA_LEN 10000
#define SEARCH_LEN 10000
#define NUM_TESTS 100000

int main() {
    uint8_t data[DATA_LEN];
    uint8_t search[SEARCH_LEN];
    uint32_t search_len = 0;
    bool reverse = false;
    srand(time(NULL));
    printf("Doing %d tests...\n", NUM_TESTS);

    // Perform NUM_TESTS searches with random starting positions
    for (int i = 0; i < NUM_TESTS; i++) {
        reverse = rand() % 2;
        search_len = (rand() % (SEARCH_LEN + 1 - SEARCH_LEN/4))+ SEARCH_LEN/4;

        // Fill data array with random values
        for (int j = 0; j < DATA_LEN; j++) {
            data[j] = rand()%256;
        }

        // Choose a random subarray of data to use as the search array
        int start_pos = rand() % (DATA_LEN - search_len + 1);
        for (int j = 0; j < search_len; j++) {
            search[j] = data[start_pos + j];
        }
        //printf("Test start_pos: %d, reverse:%d \n", start_pos, reverse);

        void* result = mem_search(data, DATA_LEN, search, search_len, reverse);


        int index = (uint8_t*)result - data;
        bool error=false;
        if (result == NULL) 
        {
            printf("Test %d: Search array not found in data\n", i);
            error=true;
        }
        else if(index != start_pos)
        {
            printf("Test %d: Search array found at index %d, expected %d\n", i, index, start_pos);
            error=true;
        }
        else if(memcmp(result, search, search_len) != 0)
        {
            printf("Test %d: Search array found at index %d, but data does not match\n", i, index);
            error=true;
        }

        if(error)
        {
            printf("start_pos:%d search len: %d, reverse: %d\n",start_pos, search_len, reverse);
        }
            //printf("Test %d: Found search array at index %d\n", i, index);
        
    }

    printf("Done!\n");
    return 0;
}