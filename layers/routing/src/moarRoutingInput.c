//
// Created by svalov on 8/16/16.
//

#include <moarRoutingInput.h>
#include <funcResults.h>

int processInputFromChannel(RouteStoredPacket_T* packet, ChannelReceiveMetadata_T* metadata, void* data, PayloadSize_T dataSize){
	if(NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == metadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == dataSize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	
	return FUNC_RESULT_FAILED;
}
