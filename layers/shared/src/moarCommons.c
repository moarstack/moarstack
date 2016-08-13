//
// Created by kryvashek on 04.07.16.
//

#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdbool.h>
#include <memory.h>

#include <moarCommons.h>
#include <funcResults.h>

static const size_t	LAYER_COMMAND_PLAIN_SIZE = sizeof( LayerCommandPlain_T );

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

    LayerCommandPlain_T commandPlain = {0};
	//read command
    ssize_t commandReadedSize = read(fd, &commandPlain, LAYER_COMMAND_PLAIN_SIZE);
    //check result
    if(-1 == commandReadedSize)
        return FUNC_RESULT_FAILED_IO; //TODO check errno
    if((long)LAYER_COMMAND_PLAIN_SIZE != commandReadedSize)
        return FUNC_RESULT_FAILED_IO;
    command->Command = commandPlain.Command;
    command->MetaSize = commandPlain.MetaSize;
    command->DataSize = commandPlain.DataSize;
	command->Data = NULL;
	command->MetaData = NULL;
    int res = FUNC_RESULT_SUCCESS;
    //if have metadata
    if(0 != commandPlain.MetaSize) {
        //create buffer
        command->MetaData = malloc(commandPlain.MetaSize);
        if (NULL == command->MetaData)
            res = FUNC_RESULT_FAILED_MEM_ALLOCATION;
        else {
            //read metadata
            ssize_t metadataReadedSize = read(fd, command->MetaData, command->MetaSize);
            //check result
            if (-1 == metadataReadedSize || commandPlain.MetaSize != metadataReadedSize) {
                res = FUNC_RESULT_FAILED_IO;
            }
        }
    }
    //if have data
    if(0 != commandPlain.DataSize) {
        //create buffer
        command->Data = malloc(commandPlain.DataSize);
        if (NULL == command->Data)
            res = FUNC_RESULT_FAILED_MEM_ALLOCATION;
        else {
            //read data
            ssize_t dataReadedSize = read(fd, command->Data, command->DataSize);
            //check result
            if (-1 == dataReadedSize || command->DataSize != dataReadedSize) {

                res = FUNC_RESULT_FAILED_IO;
            }
        }
    }
    if(FUNC_RESULT_SUCCESS != res){
        //free all memory
        free(command->Data);
        command->Data = NULL;
        free(command->MetaData);
        command->MetaData = NULL;
    }
    return res;
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
    LayerCommandPlain_T commandPlain = {0};
    commandPlain.Command = command->Command;
    commandPlain.MetaSize = command->MetaSize;
	commandPlain.DataSize = command->DataSize;
    //write command
    ssize_t writedCommandPlain = write(fd, &commandPlain, LAYER_COMMAND_PLAIN_SIZE);
    //check
    if(-1 == writedCommandPlain)
        return FUNC_RESULT_FAILED_IO; //TODO check errno
//    why this condition is true?
    if(LAYER_COMMAND_PLAIN_SIZE > writedCommandPlain)
        return FUNC_RESULT_FAILED_IO;
    //if have metadata
    if(0 != command->MetaSize)
    {
        //write metadata
        ssize_t writedMetadata = write(fd, command->MetaData, command->MetaSize);
        if(-1 == writedMetadata)
            return FUNC_RESULT_FAILED_IO; //TODO check errno
        if(command->MetaSize != writedMetadata)
            return FUNC_RESULT_FAILED_IO;
    }
    if(0 != command->DataSize)
    {
        //write data
        ssize_t writedData = write(fd, command->Data, command->DataSize);
        if(-1 == writedData)
            return FUNC_RESULT_FAILED_IO; //TODO check errno
        if(command->DataSize != writedData)
            return FUNC_RESULT_FAILED_IO;
    }
    return FUNC_RESULT_SUCCESS;
}

int SocketOpenFile( const SocketFilepath_T socketFilePath, const bool isServer, int * sockResult ) {
	struct sockaddr_un	socketFileAddress;
	int					socketValue,
						result;

	if( NULL == socketFilePath || NULL == sockResult )
		return FUNC_RESULT_FAILED;

	memset( &socketFileAddress, 0, sizeof( struct sockaddr_un ) );
	socketFileAddress.sun_family = AF_UNIX;
	strncpy( socketFileAddress.sun_path, socketFilePath, SOCKET_FILEPATH_SIZE );
	result = access( socketFileAddress.sun_path, F_OK ); // check whether file exists

	if( !isServer ) { // it is client
		if( -1 == result || -1 == access( socketFileAddress.sun_path, R_OK | W_OK ) ) // file not exists or can`t be accessed
			return FUNC_RESULT_FAILED_IO;
	} else if( 0 == result && -1 == unlink( socketFileAddress.sun_path ) ) // it is server, file exists and unable to delete it
		return FUNC_RESULT_FAILED_IO;

	socketValue = socket( AF_UNIX, SOCK_STREAM, 0 );

	if( -1 == socketValue )
		return FUNC_RESULT_FAILED_IO;

	if( isServer ) {
		int	reuse = 1;

		result = setsockopt( socketValue, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( int ) );

		if( -1 != result )
			result = bind( socketValue, ( struct sockaddr * )&socketFileAddress, sizeof( struct sockaddr_un ) );

		if( -1 != result )
			result = listen( socketValue, 1 );
	} else
		result = connect( socketValue, ( struct sockaddr * )&socketFileAddress, sizeof( struct sockaddr_un ) );

	if( -1 == result )
		return FUNC_RESULT_FAILED_IO;

	memcpy( sockResult, &socketValue, sizeof( int ) );
	return FUNC_RESULT_SUCCESS;
}

int FreeCommand( LayerCommandStruct_T * command ) {
	if( NULL == command )
		return FUNC_RESULT_FAILED_ARGUMENT;

	free( command->MetaData );
	command->MetaData = NULL;
	command->MetaSize = 0;

	free( command->Data );
	command->Data = NULL;
	command->DataSize = 0;

	return FUNC_RESULT_SUCCESS;
}

int ProcessCommand(void* layer, int fd, uint32_t event, uint32_t eventMask, CommandProcessingRule_T* rules) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL != rules)
		return FUNC_RESULT_FAILED_ARGUMENT;

	if ((event & eventMask) != 0) {
		// get command
		LayerCommandStruct_T command = {0};
		int commandRes = ReadCommand(fd, &command);
		if (FUNC_RESULT_SUCCESS != commandRes) {
			return commandRes;
		}
		int res = FUNC_RESULT_FAILED;
		for(int i=0; (i<LayerCommandType_TypesCount)
					 && (LayerCommandType_None != rules[i].CommandType)
					 && (NULL != rules[i].ProcessingRule); i++){
			if(command.Command == rules[i].CommandType){
				res = rules[i].ProcessingRule(layer,fd, &command);
			}
		}
		FreeCommand(&command);
		return res;
	}
	return FUNC_RESULT_FAILED;
}

CommandProcessingRule_T MakeProcessingRule(LayerCommandType_T type, CommandProcessor_T rule){
	CommandProcessingRule_T processingRule = {0};
	processingRule.ProcessingRule = rule;
	processingRule.CommandType = type;
	return processingRule;
}