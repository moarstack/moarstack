//
// Created by svalov on 9/13/16.
//

#include <funcResults.h>
#include "priorityQueue.h"
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <priorityQueue.h>


int pqInit(PriorityQueue_T* queue, int size, pqCompareFunc_T func, size_t keySize, size_t dataSize){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == func)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == keySize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == dataSize)
		return FUNC_RESULT_FAILED_ARGUMENT;

	if (size < 4)  // allocate at least for 4 elements
		size = 4;
	queue->Storage = (pqEntry_T *)malloc(sizeof(pqEntry_T) * size);
	if(NULL == queue->Storage)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	queue->Allocated = size;
	queue->Count = 0;

	queue->Compare = func;
	queue->DataSize = dataSize;
	queue->PrioritySize = keySize;

	return FUNC_RESULT_SUCCESS;
}
int pqClear(PriorityQueue_T* queue){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;

	while(queue->Count!=0)
		pqDequeue(queue, NULL);
	return FUNC_RESULT_SUCCESS;
}
int pqDeinit(PriorityQueue_T* queue){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int clearRes = pqClear(queue);
	free(queue->Storage);
	queue->Storage = NULL;
	queue->Allocated = 0;
	return clearRes;
}

static inline int parent(int index){
	return (index-1)/2;
}
static inline int child(int index){
	return index*2 + 1;
}

int pqLift(PriorityQueue_T* queue, int index){
	pqEntry_T item = queue->Storage[index];
	while(	index>0 &&
			queue->Compare(item.Priority, queue->Storage[parent(index)].Priority, queue->PrioritySize)>0
			){
		int prnt = parent(index);
		queue->Storage[index] = queue->Storage[prnt];
		index = prnt;
	}
	queue->Storage[index] = item;
	return index;
}
int pqSift(PriorityQueue_T* queue, int index){
	pqEntry_T item = queue->Storage[index];
	while(index<=parent(queue->Count-1)){
		int j = child(index);
		if(		j<queue->Count &&
				(queue->Compare(queue->Storage[j].Priority, queue->Storage[j+1].Priority, queue->PrioritySize)<0)
				)
			++j;
		if(queue->Compare(item.Priority, queue->Storage[j].Priority, queue->PrioritySize)>=0) break;
		queue->Storage[index] = queue->Storage[j];
		index = j;
	}
	queue->Storage[index] = item;
	return index;
}

int pqEnqueue(PriorityQueue_T* queue, void* key, void* data){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == key)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	if (queue->Count >= queue->Allocated)
	{
		queue->Allocated *= 2;
		queue->Storage = (pqEntry_T*)realloc(queue->Storage , sizeof(pqEntry_T) * queue->Allocated);
		// todo check returned type
	}

	int index = queue->Count;
	// allocate and copy data
	queue->Storage[index].Data = malloc(queue->DataSize);
	if(NULL == queue->Storage[index].Data)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	memcpy(queue->Storage[index].Data, data, queue->DataSize);

	// allocate and copy key
	queue->Storage[index].Priority = malloc(queue->PrioritySize);
	if(NULL == queue->Storage[index].Priority)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	memcpy(queue->Storage[index].Priority, key, queue->PrioritySize);

	queue->Count++;
	// lift
	int liftRes = pqLift(queue, index);
	return FUNC_RESULT_SUCCESS;
}
int pqDequeue(PriorityQueue_T* queue, void* data){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == queue->Count)
		return FUNC_RESULT_FAILED_ARGUMENT;

	if(NULL != data)
		memcpy(data, queue->Storage[0].Data, queue->DataSize);
	// free
	free(queue->Storage[0].Priority);
	queue->Storage[0].Priority = NULL;
	free(queue->Storage[0].Data);
	queue->Storage[0].Data = NULL;
	// move
	queue->Storage[0] = queue->Storage[--(queue->Count)];
	// sift down
	int siftRes = pqSift(queue,0);
	return FUNC_RESULT_SUCCESS;
}

int pqRemove(PriorityQueue_T* queue, void* data){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int index = -1;
	for(int i=0;i<queue->Count; i++){
		if(memcmp(data, queue->Storage[i].Data, queue->DataSize) == 0){
			index = i;
			break;
		}
	}
	if(-1 == index)
		return FUNC_RESULT_SUCCESS;

	// swap with last
	pqEntry_T tmp = queue->Storage[index];
	queue->Storage[index] = queue->Storage[queue->Count-1];
	queue->Storage[queue->Count-1] = tmp;

	// free memory
	free(tmp.Priority);
	free(tmp.Data);
	// decrement
	queue->Count--;
	int liftRes = pqLift(queue, index);
	int siftRes = pqSift(queue, index);
	return FUNC_RESULT_SUCCESS;
}

int pqTop(PriorityQueue_T* queue, void* data){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	void* dataPtr = pqTopData(queue);
	if(NULL != dataPtr)
		return FUNC_RESULT_FAILED;
	memcpy(data, dataPtr, queue->DataSize);

	return FUNC_RESULT_SUCCESS;
}
void* pqTopData(PriorityQueue_T* queue){
	if(NULL == queue)
		return NULL;
	if(0 == queue->Count)
		return NULL;
	return queue->Storage[0].Data;
}
void* pqTopPriority(PriorityQueue_T* queue){
	if(NULL == queue)
		return NULL;
	if(0 == queue->Count)
		return NULL;
	return queue->Storage[0].Priority;
}

int pqChangePriority(PriorityQueue_T* queue, void* data, void* newKey){
	if(NULL == queue)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == newKey)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int index = -1;
	for(int i=0;i<queue->Count; i++){
		if(memcmp(data, queue->Storage[i].Data, queue->DataSize) == 0){
			index = i;
			break;
		}
	}
	if(-1 == index)
		return FUNC_RESULT_FAILED;

	memcpy(queue->Storage[index].Priority, newKey, queue->PrioritySize);
	int liftRes = pqLift(queue, index);
	int siftRes = pqSift(queue, index);
	return FUNC_RESULT_SUCCESS;
}