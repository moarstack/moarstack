//
// Created by svalov on 9/26/16.
//

#ifndef MOARSTACK_QUEUE_H
#define MOARSTACK_QUEUE_H

#endif //MOARSTACK_QUEUE_H
#include <stdlib.h>

typedef struct QueueListEntry_T QueueListEntry_T;

struct QueueListEntry_T {
	void* Data;
	QueueListEntry_T* Next;
};

typedef struct{
	int Count;
	QueueListEntry_T* Head;
	QueueListEntry_T* Tail;
	size_t DataSize;
}Queue_T;

int queueInit(Queue_T* queue, size_t dataSize);
int queueDenit(Queue_T* queue);
int queueClear(Queue_T* queue);
int queueEnqueue(Queue_T* queue, void* data);
int queueDequeue(Queue_T* queue, void* data);
int queuePeek(Queue_T* queue, void* data);
void* queuePeekPtr(Queue_T* queue);