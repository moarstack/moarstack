//
// Created by svalov on 9/26/16.
//

#include <funcResults.h>
#include <queue.h>
#include <memory.h>

int queueInit(Queue_T* queue, size_t dataSize){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == dataSize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	queue->Count = 0;
	queue->DataSize = dataSize;
	queue->Head = NULL;
	queue->Tail = NULL;
	return FUNC_RESULT_SUCCESS;
}
int queueDeinit(Queue_T* queue){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = queueClear(queue);
	return res;
}
int queueClear(Queue_T* queue){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	while(queue->Count != 0){
		int res = queueDequeue(queue, NULL);
		if(FUNC_RESULT_SUCCESS != res)
			return res;
	}
	return FUNC_RESULT_SUCCESS;
}
int queueEnqueue(Queue_T* queue, void* data){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	QueueListEntry_T* entry = (QueueListEntry_T*)malloc(sizeof(QueueListEntry_T));
	if(NULL == entry)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	entry->Data = malloc(queue->DataSize);
	if(NULL == entry->Data) {
		free(entry);
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	}
	memcpy(entry->Data, data, queue->DataSize);

	entry->Next = NULL;
	//if(NULL == queue->Head)
	//	queue->Head = entry;
	if(NULL != queue->Tail)
		queue->Tail->Next = entry;

	queue->Tail = entry;

	if(NULL == queue->Head)
		queue->Head = entry;

	queue->Count++;
	return FUNC_RESULT_SUCCESS;
}
int queueDequeue(Queue_T* queue, void* data) {
	if (NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (queue->Count == 0)
		return FUNC_RESULT_FAILED;

	if(NULL == queue->Head)
		return FUNC_RESULT_FAILED;
	QueueListEntry_T *entry = queue->Head;

	queue->Head = entry->Next;
	if(NULL == queue->Head)
		queue->Tail = NULL;

	if(NULL != data)
		memcpy(data, entry->Data, queue->DataSize);
	free(entry->Data);
	free(entry);

	queue->Count--;
	return FUNC_RESULT_SUCCESS;
}
int queuePeek(Queue_T* queue, void* data){
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	void* ptr = queuePeekPtr(queue);

	if(NULL == ptr)
		return FUNC_RESULT_FAILED;
	memcpy(data, ptr, queue->DataSize);

	return FUNC_RESULT_SUCCESS;
}
void* queuePeekPtr(Queue_T* queue){
	if (NULL == queue)
		return NULL;
	if (queue->Count == 0)
		return NULL;
	if(NULL == queue->Head)
		return NULL;
	return queue->Head->Data;
}

int queuePushToFront(Queue_T* queue, void* data){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	QueueListEntry_T* entry = (QueueListEntry_T*)malloc(sizeof(QueueListEntry_T));
	if(NULL == entry)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	entry->Data = malloc(queue->DataSize);
	if(NULL == entry->Data) {
		free(entry);
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	}
	memcpy(entry->Data, data, queue->DataSize);

	entry->Next = queue->Head;
	queue->Head = entry;
	if(NULL == queue->Tail)
		queue->Tail = entry;

	queue->Count++;
	return FUNC_RESULT_SUCCESS;
}
int queueIterator(Queue_T* queue, QueueIterator_T* iterator){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == iterator)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == queue->Count || NULL == queue->Head) {
		iterator->Entry.Data = NULL;
		iterator->Entry.Next= NULL;
		return FUNC_RESULT_SUCCESS;
	}
	iterator->DataSize = queue->DataSize;
	iterator->Entry = *queue->Head;
	return FUNC_RESULT_SUCCESS;
}
int queueIteratorNext(QueueIterator_T* iterator){
	if(NULL == iterator)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == iterator->Entry.Data)
		return FUNC_RESULT_FAILED;
	if(NULL == iterator->Entry.Next)
		iterator->Entry.Data = NULL;
	else
		iterator->Entry = *(iterator->Entry.Next);
	return FUNC_RESULT_SUCCESS;
}
int queueIteratorData(QueueIterator_T* iterator, void* data){
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == iterator->DataSize)
		return FUNC_RESULT_FAILED_ARGUMENT;

	void* val = queueIteratorDataPtr(iterator);
	if(NULL == val)
		return FUNC_RESULT_FAILED;
	memcpy(data,val,iterator->DataSize);
	
	return FUNC_RESULT_SUCCESS;
}
void* queueIteratorDataPtr(QueueIterator_T* iterator){
	if(NULL == iterator)
		return NULL;
	return iterator->Entry.Data;
}
bool queueIteratorIsEnd(QueueIterator_T* iterator){
	if(NULL == iterator)
		return true;
	if(NULL == iterator->Entry.Data)
		return true;
	return false;
}