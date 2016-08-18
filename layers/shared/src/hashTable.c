//
// Created by svalov on 8/18/16.
//

#include <funcResults.h>
#include <hashTable.h>
#include <memory.h>
#include "hashTable.h"

int hashInit(hashTable_T* table, hashFunc_T function, int storageSize, size_t keySize, size_t dataSize){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == function)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == storageSize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == keySize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == dataSize)
		return FUNC_RESULT_FAILED_ARGUMENT;

	table->DataSize = dataSize;
	table->KeySize = keySize;
	table->HashFunction = function;
	table->StorageSize = storageSize;
	table->Count = 0;

	size_t totalStorageSize = sizeof(hashEntry_T*)*storageSize;
	table->Table = malloc(totalStorageSize);
	if(NULL == table->Table)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	memset(table->Table,0, totalStorageSize);

	return FUNC_RESULT_SUCCESS;
}
int hashFree(hashTable_T* table){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 != table->Count)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//
	return FUNC_RESULT_SUCCESS;
}
int hashAdd(hashTable_T* table, void* key, void* data){
	
}
int hashRemove(hashTable_T* table, void* key);
int hashGet(hashTable_T* table, void* key, void*);