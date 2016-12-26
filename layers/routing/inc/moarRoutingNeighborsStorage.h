//
// Created by svalov on 9/14/16.
//

#ifndef MOARSTACK_MOARROUTINGNEIGHBORSSTORAGE_H
#define MOARSTACK_MOARROUTINGNEIGHBORSSTORAGE_H

#include <moarRouting.h>
#include <hashTable.h>

#define STORAGE_HASH_SIZE 41

#pragma pack(push, 1)

typedef struct{
	RouteAddr_T Address;
}RoutingNeighborInfo_T;

typedef struct{
	hashTable_T Storage;
	int Count;
}RoutingNeighborsStorage_T;

#pragma pack(pop)

int storageInit(RoutingNeighborsStorage_T* storage);
int storageDeinit(RoutingNeighborsStorage_T* storage);

int storageAdd(RoutingNeighborsStorage_T* storage, RoutingNeighborInfo_T* info);

int storageGet(RoutingNeighborsStorage_T* storage, RouteAddr_T* address, RoutingNeighborInfo_T* info);
RoutingNeighborInfo_T* storageGetPtr(RoutingNeighborsStorage_T* storage, RouteAddr_T* address);
int storageRemove(RoutingNeighborsStorage_T* storage, RouteAddr_T* address);

#ifdef HASH_ENABLE_ITERATOR
int storageIterator(RoutingNeighborsStorage_T* storage, hashIterator_T* iterator);
RoutingNeighborInfo_T*  storageIteratorData(hashIterator_T* iterator);
#endif
#endif //MOARSTACK_MOARROUTINGNEIGHBORSSTORAGE_H
