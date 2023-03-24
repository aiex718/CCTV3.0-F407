#ifndef BOOLEAN_H
#define BOOLEAN_H

#ifdef __STDC_VERSION__
#include <stdbool.h>
//#warning "Using stdbool.h"
#endif

#ifndef bool
typedef enum {
    false=0, true=1
} bool;

// #define bool uint8_t
// #define ture 1
// #define false 0
// #define __bool_true_false_are_defined 1
#endif


#endif
