//
// Created by kryvashek on 04.07.16.
//

#include <moarCommons.h>
#include <unistd.h>
#include <stdlib.h>
#include "moarCommons.h"
#include

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

// read command from socket
int ReadCommand(int fd, LayerCommandStruct_T* command){
    if(NULL == command)
        return 1;
    if(fd <= 0)
        return 1;

    LayerCommandPlain_T commandPlain;
    size_t commandPlainSize = sizeof(LayerCommandPlain_T);
    //read command
    ssize_t commandReadedSize = read(fd,&commandPlain, commandPlainSize);
    //check result
    if(-1 == commandReadedSize)
        return 2; //TODO check errno
    if(commandReadedSize != commandPlainSize)
        return 3;
    command->Command = commandPlain.Command;
    command->MetaSize = commandPlain.MetaSize;
    //if have metadata
    if(0 != commandPlain.MetaSize){
        //create buffer
        uint8_t *buffer;
        buffer = (uint8_t *)malloc(commandPlain.MetaSize);
        if(NULL == buffer)
            return 4;
        //read metadata
        ssize_t metadataReadedSize = read(fd, buffer, commandPlain.MetaSize);
        //check result
        if(-1 == metadataReadedSize)
            return 2; //TODO check errno
        if(metadataReadedSize != commandPlain.MetaSize)
            return 3;
        command->MetaData = (void *)buffer;
    }
    return 0;
}

// write command to socket
int WriteCommand(int fd, LayerCommandStruct_T* command){

    return 0;
}