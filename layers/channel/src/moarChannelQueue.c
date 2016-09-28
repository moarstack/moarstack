//
// Created by svalov on 7/28/16.
//

#include <funcResults.h>
#include <moarChannelPrivate.h>
#include <stdlib.h>
#include <priorityQueue.h>
#include "moarChannelQueue.h"

int compareMoarTime(void* t1, void* t2, size_t size){
	if(NULL == t1 || NULL == t2 || 0 == size)
		return 0;
	moarTime_T* time1 = (moarTime_T*)t1;
	moarTime_T* time2 = (moarTime_T*)t2;
	int res = timeCompare(*time1, *time2);
	return -res;
}

int messageQueueInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = pqInit(&(layer->MessageQueue), MESSAGE_QUEUE_SIZE, compareMoarTime, sizeof(moarTime_T), sizeof(ChannelMessageEntry_T));
	return res;
}
int messageDequeue(ChannelLayer_T* layer, ChannelMessageEntry_T* entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = pqDequeue(&(layer->MessageQueue), entry);
	return res;
}
int messageEnqueue(ChannelLayer_T* layer, ChannelMessageEntry_T* entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = pqEnqueue(&(layer->MessageQueue),&(entry->ProcessingTime), entry);
	return res;
}
int messagePeek(ChannelLayer_T* layer, ChannelMessageEntry_T** entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;
	ChannelMessageEntry_T* top = pqTopData(&(layer->MessageQueue));
	if(NULL == top)
		return FUNC_RESULT_FAILED;
	*entry = top;
	return FUNC_RESULT_SUCCESS;
}