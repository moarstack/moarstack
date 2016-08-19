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

#ifdef HASH_ENABLE_ITERATOR
	table->Last = NULL;
#endif

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
	free(table->Table);
	table->Table = NULL;
	return FUNC_RESULT_SUCCESS;
}
int hashAdd(hashTable_T* table, void* key, void* data){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == key)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int hash = table->HashFunction(key, table->KeySize);
	int bin = hash % table->StorageSize;

	hashEntry_T* entry = malloc(sizeof(hashEntry_T));
	if(NULL == entry)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	entry->HashValue = hash;
	entry->Next = table->Table[bin];

	//allocate
	entry->Key = malloc(table->KeySize);
	entry->Data = malloc(table->DataSize);
	if(NULL == entry->Key || NULL == entry->Data){
		free(entry->Key);
		free(entry->Data);
		free(entry);
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	}
	//copy
	memcpy(entry->Key, key, table->KeySize);
	memcpy(entry->Data, data, table->DataSize);

#ifdef HASH_ENABLE_ITERATOR
	entry->ListPrev = table->Last;
	if(NULL != table->Last)
		table->Last->ListNext = entry;
	entry->ListNext = NULL;
	table->Last = entry;
#endif

	table->Table[bin] = entry;
	table->Count++;
	return FUNC_RESULT_SUCCESS;
}
int hashRemove(hashTable_T* table, void* key){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == key)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int hash = table->HashFunction(key, table->KeySize);
	int bin = hash % table->StorageSize;

	hashEntry_T** entry = table->Table+bin;

	while(NULL != *entry){
		hashEntry_T* cEntry = *entry;
		if(cEntry->HashValue == hash){
			// compare key
			int compare = memcmp(cEntry->Key, key, table->KeySize);
			//found
			if(0 == compare){
#ifdef HASH_ENABLE_ITERATOR
				//remove from list
				if(NULL != cEntry->ListPrev)
					cEntry->ListPrev->ListNext = cEntry->ListNext;
				if(NULL != cEntry->ListNext)
					cEntry->ListNext->ListPrev = cEntry->ListPrev;
				if(table->Last == cEntry)
					table->Last = cEntry->ListPrev;
#endif

				//remove
				free(cEntry->Key);
				free(cEntry->Data);
				*entry = cEntry->Next;
				free(cEntry);
				table->Count--;
				return FUNC_RESULT_SUCCESS;
			}
		}
		entry = &(cEntry->Next);
	}
	return FUNC_RESULT_SUCCESS;
}
int hashGet(hashTable_T* table, void* key, void* data){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == key)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int hash = table->HashFunction(key, table->KeySize);
	int bin = hash % table->StorageSize;

	hashEntry_T** entry = table->Table+bin;

	while(NULL != *entry){
		hashEntry_T* cEntry = *entry;
		if(cEntry->HashValue == hash){
			// compare key
			int compare = memcmp(cEntry->Key, key, table->KeySize);
			// found
			if(0 == compare){
				// copy
				memcpy(data, cEntry->Data, table->DataSize);
				return FUNC_RESULT_SUCCESS;
			}
		}
		entry = &(cEntry->Next);
	}
	return FUNC_RESULT_FAILED;
}

bool hashContain(hashTable_T* table, void* key){
	if(NULL == table)
		return false;
	if(NULL == key)
		return false;

	int hash = table->HashFunction(key, table->KeySize);
	int bin = hash % table->StorageSize;

	hashEntry_T** entry = table->Table+bin;

	while(NULL != *entry){
		hashEntry_T* cEntry = *entry;
		if(cEntry->HashValue == hash){
			// compare key
			int compare = memcmp(cEntry->Key, key, table->KeySize);
			// found
			if(0 == compare){
				// return
				return true;
			}
		}
		entry = &(cEntry->Next);
	}
	return false;
}

#ifdef HASH_ENABLE_ITERATOR
hashIterator_T hashGetIterator(hashTable_T* table){
	if(NULL == table)
		return NULL;
	return table->Last;
}

hashIterator_T hashIteratorNext(hashIterator_T item){
	if(NULL == item)
		return NULL;
	return item->ListPrev;
}

void* hashIteratorData(hashIterator_T item){
	if(NULL == item)
		return NULL;
	return item->Data;
}

void* hashIteratorKey(hashIterator_T item){
	if(NULL == item)
		return NULL;
	return item->Key;
}
#endif