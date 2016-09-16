//
// Created by svalov on 9/14/16.
//

#include <moarRoutingNeighborsStorage.h>
#include <funcResults.h>
#include <hashFunc.h>
#include <hashTable.h>

int storageInit(RoutingNeighborsStorage_T* storage){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int hashRes = hashInit(&(storage->Storage),hashRoutingAddress, STORAGE_HASH_SIZE, sizeof(RouteAddr_T), sizeof(RoutingNeighborInfo_T));
	return hashRes;
}
int storageDeinit(RoutingNeighborsStorage_T* storage){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int hashRes = hashFree(&(storage->Storage));
	return hashRes;
}

int storageAdd(RoutingNeighborsStorage_T* storage, RoutingNeighborInfo_T* info){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == info)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int hashRes = hashAdd(&(storage->Storage), &(info->Address), info);
	return hashRes;
}

int storageGet(RoutingNeighborsStorage_T* storage, RouteAddr_T* address, RoutingNeighborInfo_T* info)
{
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == info)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int hashRes = hashGet(&(storage->Storage), address, info);
	return hashRes;
}
RoutingNeighborInfo_T* storageGetPtr(RoutingNeighborsStorage_T* storage, RouteAddr_T* address)
{
	if(NULL == storage)
		return NULL;
	if(NULL == address)
		return NULL;

	RoutingNeighborInfo_T* neighbor = (RoutingNeighborInfo_T*)hashGetPtr(&(storage->Storage), address);
	return neighbor;
}
int storageRemove(RoutingNeighborsStorage_T* storage, RouteAddr_T* address){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int hashRes = hashRemove(&(storage->Storage), address);
	return hashRes;
}
#ifdef HASH_ENABLE_ITERATOR
int storageIterator(RoutingNeighborsStorage_T* storage, hashIterator_T* iterator){
	if(NULL == storage)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == iterator)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int iterRes = hashIterator(&(storage->Storage), iterator);

	return iterRes;
}
RoutingNeighborInfo_T*  storageIteratorData(hashIterator_T* iterator){
	if(NULL == iterator)
		return NULL;
	return (RoutingNeighborInfo_T*)hashIteratorData(iterator);
}

#endif
