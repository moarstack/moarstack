//
// Created by svalov on 8/16/16.
//

#include <stddef.h>
#include <funcResults.h>
#include <memory.h>
#include <moarRoutingMessageId.h>
#include <stdlib.h>
#include <moarTime.h>
#include "moarRoutingMessageId.h"
#include <hashFunc.h>
uint32_t rmidHash(void* id, size_t size){
	return hashBytesEx(id, size, 0xf4e20f);
}
bool rmidEqual( RoutingMessageId_T * one, RoutingMessageId_T * two ){
	if(NULL == one || NULL == two)
		return false;
	else if( one == two ) // pointers are equal
		return true;
	else
		return (0 == memcmp( one, two, ROUTING_MESSAGE_ID_SIZE));
}
int rmidGenerate( RoutingMessageId_T * identifier){
	if(NULL == identifier)
		return FUNC_RESULT_FAILED_ARGUMENT;

	srand((unsigned int)timeGetCurrent());
	for(int i=0; i<ROUTING_MESSAGE_ID_SIZE; i++)
		identifier->Data[i] = (uint8_t)(rand() & 0xFF);

	return FUNC_RESULT_SUCCESS;
}