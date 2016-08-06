//
// Created by svalov on 7/28/16.
//

#include <funcResults.h>
#include <moarChannelPrivate.h>
#include <stdlib.h>
#include "moarChannelMessageQueue.h"

int queueInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = CreateList(&(layer->MessageQueue));
	return res;
}
int dequeueMessage(ChannelLayer_T* layer, ChannelMessageEntry_T* entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;
	LinkedListItem_T* item = PrevElement(&(layer->MessageQueue));
	if(NULL != item) {
		*entry = *((ChannelMessageEntry_T *) item);
		item = DeleteElement(item);
		return FUNC_RESULT_SUCCESS;
	}
	return FUNC_RESULT_FAILED;
}
int enqueueMessage(ChannelLayer_T* layer, ChannelMessageEntry_T* entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//allocate
	ChannelMessageEntry_T* allocated = malloc(sizeof(ChannelMessageEntry_T));
	if(NULL == allocated)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	*allocated = *entry;
	int res = AddNext(&(layer->MessageQueue), allocated);
	return res;
}
int peekMessage(ChannelLayer_T* layer, ChannelMessageEntry_T** entry){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;
	LinkedListItem_T* item = PrevElement(&(layer->MessageQueue));
	if(NULL != item && NULL != item->Data){
		*entry = (ChannelMessageEntry_T*)item->Data;
		return FUNC_RESULT_SUCCESS;
	}
	return FUNC_RESULT_FAILED;
}