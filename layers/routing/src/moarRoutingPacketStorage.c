//
// Created by svalov on 9/14/16.
//

#include "moarRoutingPacketStorage.h"
#include <funcResults.h>
#include <hashFunc.h>
#include <moarMessageId.h>
#include <moarRoutingMessageId.h>

int psInit(PacketStorage_T* storage){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;



	return FUNC_RESULT_SUCCESS;
}