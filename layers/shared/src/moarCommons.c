//
// Created by kryvashek on 04.07.16.
//

#include <moarCommons.h>
#include <unistd.h>
#include <stdlib.h>
#include "moarCommons.h"
#include "funcResults.h"

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
    //check args
    if(NULL == command)
        return FUNC_RESULT_FAILED_ARGUMENT;
    if(fd <= 0)
        return FUNC_RESULT_FAILED_ARGUMENT;

    LayerCommandPlain_T commandPlain;

    //read command
    ssize_t commandReadedSize = read(fd, &commandPlain, LAYER_COMMAND_PLAIN_SIZE);
    //check result
    if(-1 == commandReadedSize)
        return FUNC_RESULT_FAILED_IO; //TODO check errno
    if(LAYER_COMMAND_PLAIN_SIZE != commandReadedSize)
        return FUNC_RESULT_FAILED_IO;
    command->Command = commandPlain.Command;
    command->MetaSize = commandPlain.MetaSize;
    //if have metadata
    if(0 != commandPlain.MetaSize){
        //create buffer
		command->MetaData = malloc(commandPlain.MetaSize);
        if(NULL == command->MetaData)
            return FUNC_RESULT_FAILED_MEM_ALLOCATION;
        //read metadata
        ssize_t metadataReadedSize = read(fd, command->MetaData, commandPlain.MetaSize);
        //check result
        if(-1 == metadataReadedSize)
            return FUNC_RESULT_FAILED_IO; //TODO check errno
        if(commandPlain.MetaSize != metadataReadedSize)
            return FUNC_RESULT_FAILED_IO;
    }
    return FUNC_RESULT_SUCCESS;
}

// write command to socket
int WriteCommand(int fd, LayerCommandStruct_T* command){
    //check args
    if(NULL == command)
        return FUNC_RESULT_FAILED_ARGUMENT;
    if(fd <= 0)
        return FUNC_RESULT_FAILED_ARGUMENT;
    if(0 != command->MetaSize && NULL == command->MetaData)
        return FUNC_RESULT_FAILED_ARGUMENT;

    //fill palin
    LayerCommandPlain_T commandPlain;
    commandPlain.Command = command->Command;
    commandPlain.MetaSize = command->MetaSize;

    //write command
    ssize_t writedCommandPlain = write(fd, &commandPlain, LAYER_COMMAND_PLAIN_SIZE);
    //check
    if(-1 != writedCommandPlain)
        return FUNC_RESULT_FAILED_IO; //TODO check errno
//    why this condition is true?
    if(LAYER_COMMAND_PLAIN_SIZE > writedCommandPlain)
        return FUNC_RESULT_FAILED_IO;
    //if have metadata
    if(0 != command->MetaSize)
    {
        //write metadata
        ssize_t writedMetadata = write(fd, command->MetaData, command->MetaSize);
        if(-1 != writedMetadata)
            return FUNC_RESULT_FAILED_IO; //TODO check errno
        if(command->MetaSize != writedMetadata)
            return FUNC_RESULT_FAILED_IO;
    }
    return FUNC_RESULT_SUCCESS;
}