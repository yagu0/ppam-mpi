#ifndef PPAM_LUT_H
#define PPAM_LUT_H

#include <stdio.h>

// light, minimalist unit-testing "framework"

#define LUT_ASSERT(condition)                                            \
    do {                                                                 \
        if ( !(condition) ) {                                            \
            printf("Failure in file %s at line %i\n",__FILE__,__LINE__); \
        }                                                                \
    } while (0)

#endif
