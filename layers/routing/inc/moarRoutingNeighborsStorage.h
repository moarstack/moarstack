//
// Created by svalov on 9/14/16.
//

#ifndef MOARSTACK_MOARROUTINGNEIGHBORSSTORAGE_H
#define MOARSTACK_MOARROUTINGNEIGHBORSSTORAGE_H

#include <moarRoutingPrivate.h>
#include <hashTable.h>

#define STORAGE_HASH_SIZE 41

typedef struct{
	RouteAddr_T Address;
}RoutingNeighborInfo_T;

typedef struct{

}RoutingNeighborsStorage_T;

int storageInit(RoutingNeighborsStorage_T* storage);
int storageDeinit(RoutingNeighborsStorage_T* storage);
int storageAdd(RoutingNeighborsStorage_T* storage, RoutingNeighborInfo_T* info);
int storageGet(RoutingNeighborsStorage_T* storage, RouteAddr_T* address, RoutingNeighborInfo_T* info);
RoutingNeighborInfo_T* storageGetPtr(RoutingNeighborsStorage_T* storage, RouteAddr_T* address);
int storageRemove(RoutingNeighborsStorage_T* storage, RouteAddr_T* address);
int storageIterator(RoutingNeighborsStorage_T* storage, hashIterator_T* iterator);

#endif //MOARSTACK_MOARROUTINGNEIGHBORSSTORAGE_H
