//
// Created by kryvashek on 04.07.16.
//

#include "moarCommons.h"

// function to use if bytes order needs to be changed
// do nothing if size = 0 or input = NULL
// save bytes in the same memory if output = NULL or output = input
void ChangeBytesOrder( void * output, const void * input, const size_t size ) {
    uint8_t * one = ( uint8_t * )input,
            * two;

    if( 0 == size || NULL == input )
        return;

    else if( NULL == output || input == output ) {
        uint8_t temp_byte;

        two = ( uint8_t * )( input + size );

        while( two != one ) {
            temp_byte = *one;
            *one = *two;
            *two = temp_byte;
            one++;
            two--;
        }
    } else {
        two = ( uint8_t * )( output + size );

        while( two != input ) {
            *two = *one;
            one++;
            two--;
        }
    }
}