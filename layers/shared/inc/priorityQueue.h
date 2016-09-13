//
// Created by svalov on 9/13/16.
//

#ifndef MOARSTACK_PRIORITYQUEUE_H
#define MOARSTACK_PRIORITYQUEUE_H

//compare function
typedef int (* pqCompareFunc_T)(void* key1, void* key2, size_t size);


typedef struct{
	void* Data;
	void* Priority;
} pqEntry_T;

typedef struct {
	pqEntry_T* Storage; //array of pq_element-s
	int Count;     // number of elements in PQ
	int Allocated; 		// number of elements memory was allocated for
	size_t PrioritySize;
	size_t DataSize;
	pqCompareFunc_T Compare;
}PriorityQueue_T;

int pqInit(PriorityQueue_T* queue, int size, pqCompareFunc_T func, size_t keySize, size_t dataSize);
int pqDeinit(PriorityQueue_T* queue);
int pqEnqueue(PriorityQueue_T* queue, void* key, void* data); //copy data and key
int pqDequeue(PriorityQueue_T* queue, void* data); // copy data and free memory
int pqTop(PriorityQueue_T* queue, void* data); // copy data
void* pqTopPtr(PriorityQueue_T* queue); // pointer to top data
int pqChangePriotity(PriorityQueue_T* queue, void* data, void* newKey); //change priority

#endif //MOARSTACK_PRIORITYQUEUE_H
