//
// Created by svalov on 7/28/16.
//

#include <funcResults.h>
#include <moarChannelPrivate.h>
#include <stdlib.h>
#include "moarChannelQueue.h"

int messageQueueInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = queueInit(&(layer->MessageQueue), sizeof(ChannelMessageEntry_T));
	return res;
}
int dequeueMessage(ChannelLayer_T* layer, ChannelMessageEntry_T* entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = queueDequeue(&(layer->MessageQueue), entry);
	return res;
}
int enqueueMessage(ChannelLayer_T* layer, ChannelMessageEntry_T* entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = queueEnqueue(&(layer->MessageQueue), entry);
	return res;
}
int peekMessage(ChannelLayer_T* layer, ChannelMessageEntry_T** entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;
	ChannelMessageEntry_T* top = queuePeekPtr(&(layer->MessageQueue));
	if(NULL == top)
		return FUNC_RESULT_FAILED;
	*entry = top;
	return FUNC_RESULT_SUCCESS;
}