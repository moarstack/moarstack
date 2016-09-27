//
// Created by svalov on 9/26/16.
//

#ifndef MOARSTACK_QUEUE_H
#define MOARSTACK_QUEUE_H


#include <stdlib.h>
#include <stdbool.h>

typedef struct QueueListEntry_T QueueListEntry_T;

struct QueueListEntry_T {
	void* Data;
	QueueListEntry_T* Next;
};

typedef struct{
	struct QueueListEntry_T;
	size_t DataSize;
}QueueIterator_T;

typedef struct{
	int Count;
	QueueListEntry_T* Head;
	QueueListEntry_T* Tail;
	size_t DataSize;
}Queue_T;

int queueInit(Queue_T* queue, size_t dataSize);
int queueDeinit(Queue_T* queue);
int queueClear(Queue_T* queue);
int queueEnqueue(Queue_T* queue, void* data);
int queueDequeue(Queue_T* queue, void* data);
int queuePeek(Queue_T* queue, void* data);
void* queuePeekPtr(Queue_T* queue);

int queuePushToFront(Queue_T* queue, void* data);
int queueIterator(Queue_T* queue, QueueIterator_T* iterator);
int queueIteratorNext(QueueIterator_T* iterator);
int queueIteratorData(QueueIterator_T* iterator, void* data);
int queueIteratorDataPtr(QueueIterator_T* iterator);
bool queueIteratorIsEnd(QueueIterator_T* iterator);
#endif //MOARSTACK_QUEUE_H