//
// Created by kryvashek on 04.07.16.
//

#include "moarCommons.h"

// function to use if bytes order needs to be changed
// do nothing if size = 0 or input = NULL
// save bytes in the same memory if output = NULL or output = input
void MoarChangeOrder( void * output, const void * input, const size_t size ) {
    uint8_t temp_byte, * from_bgn, * from_end;
    size_t  count = size;

    if( 0 == size || NULL == input )
        return;
    else if( NULL == output || input == output ) {
        output = ( void * ) input;
        count /= 2;
    }

    from_bgn = ( uint8_t * )output;
    from_end = ( uint8_t * )( input + size );

    while( count > 0 ) {
        temp_byte = *from_bgn;
        *from_bgn = *from_end;
        *from_end = temp_byte;
        from_bgn++;
        from_end--;
        count--;
    }
}

#include "moarCommons.h"
