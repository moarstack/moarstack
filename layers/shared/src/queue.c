//
// Created by svalov on 9/26/16.
//

#include <funcResults.h>
#include <queue.h>

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
int queueEnqueue(Queue_T* queue, void* data){}
int queueDequeue(Queue_T* queue, void* data){}
int queuePeek(Queue_T* queue, void* data){}
void* queuePeekPtr(Queue_T* queue){}