//
// Created by svalov on 8/18/16.
//

#include <funcResults.h>
#include <hashTable.h>
#include <memory.h>
#include <stdio.h>

bool checkEquality(hashEntry_T *entry, hashVal_T hash, void *key, size_t size, equalFunc_T equal){
	if(NULL == entry)
		return false;
	if(NULL == key)
		return false;
	if(0 == size)
		return false;
	hashVal_T value = entry->HashValue;
	if(value == hash) {
		// compare key
		int compare;
		compare = equal(entry->Key, key, size);
		return (0 == compare);
	}
	return false;
}
hashEntry_T** searchEntry(hashTable_T* table, void* key, void* data){
	if(NULL == table)
		return NULL;
	if(NULL == key)
		return NULL;
	hashVal_T hash = table->HashFunction(key, table->KeySize);
	uint32_t bin = hash % table->StorageSize;
	hashEntry_T** entry = table->Table+bin;
	while(NULL != *entry){
		hashEntry_T* cEntry = *entry;
		if(checkEquality(cEntry, hash, key, table->KeySize, table->EqualFunction)){
			if(NULL != data) {
				int cmp = memcmp(cEntry->Data, data, table->DataSize);
				if(0 == cmp)
					return entry;
			} else
				return entry;
		}
		entry = &(cEntry->Next);
	}
	return NULL;
}

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
	table->EqualFunction = memcmp;
	table->DataFreeFunction = free;
	table->KeyFreeFunction = free;
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
int hashClear(hashTable_T* table){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;

	for(int i=0; i<table->StorageSize;i++){
		while(table->Table[i]!=NULL){
			// remove from list
			hashEntry_T* current = table->Table[i];
			table->Table[i] = current->Next;
		 	// free memory
			table->KeyFreeFunction(current->Key);
			table->DataFreeFunction(current->Data);
			free(current);
		}
	}
	table->Count = 0;
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

	hashVal_T hash = table->HashFunction(key, table->KeySize);
	uint32_t bin = hash % table->StorageSize;

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
	return hashRemoveExact(table, key, NULL);
}

int hashRemoveExact(hashTable_T* table, void* key, void* value){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == key)
		return FUNC_RESULT_FAILED_ARGUMENT;
	hashEntry_T** entry = searchEntry(table, key, value);
	if(NULL != entry) {
		hashEntry_T* cEntry = *entry;
#ifdef HASH_ENABLE_ITERATOR
		//remove from list
		if (NULL != cEntry->ListPrev)
			cEntry->ListPrev->ListNext = cEntry->ListNext;
		if (NULL != cEntry->ListNext)
			cEntry->ListNext->ListPrev = cEntry->ListPrev;
		if (table->Last == cEntry)
			table->Last = cEntry->ListPrev;
#endif
		//remove
		table->KeyFreeFunction(cEntry->Key);
		table->DataFreeFunction(cEntry->Data);

		*entry = cEntry->Next;
		free(cEntry);
		table->Count--;
		return FUNC_RESULT_SUCCESS;
	}
	return FUNC_RESULT_SUCCESS;
}

void* hashGetPtr(hashTable_T* table, void* key){
	if(NULL == table)
		return NULL;
	if(NULL == key)
		return NULL;

	hashEntry_T** entry = searchEntry(table, key, NULL);

	if(NULL != entry) {
		hashEntry_T* cEntry = *entry;
		return cEntry->Data;
	}
	return NULL;
}
int hashGet(hashTable_T* table, void* key, void* data){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == key)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	hashEntry_T** entry = searchEntry(table, key, NULL);
	if(NULL != entry) {
			hashEntry_T* cEntry = *entry;
			// copy
			memcpy(data, cEntry->Data, table->DataSize);
			return FUNC_RESULT_SUCCESS;
	}
	return FUNC_RESULT_FAILED;
}
bool hashContain(hashTable_T* table, void* key){
	return hashContainExact(table, key ,NULL);
}

bool hashContainExact(hashTable_T* table, void* key, void* data){
	if(NULL == table)
		return false;
	if(NULL == key)
		return false;

	hashEntry_T** entry = searchEntry(table, key, data);
	if(NULL != entry)
		return true;
	return false;
}

#ifdef HASH_ENABLE_ITERATOR
int hashGetFirst(hashTable_T* table, void* key, hashIterator_T* iterator){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == iterator)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == key)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int hash = table->HashFunction(key, table->KeySize);
	int bin = hash % table->StorageSize;

	hashEntry_T** entry = searchEntry(table, key, NULL);
	if(NULL != entry) {
		hashEntry_T *cEntry = *entry;
		// fill iterator
		iterator->Item = cEntry;
		iterator->Compare = true;
		iterator->KeySize = table->KeySize;
		iterator->Key = malloc(iterator->KeySize);
		iterator->EqualFunction = table->EqualFunction;
		if (NULL == iterator->Key)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		memcpy(iterator->Key, key, iterator->KeySize);
		iterator->HashValue = hash;
		//next fill
		iterator->NextItem = iterator->Item!=NULL ? iterator->Item->Next : NULL;
		// return sucess
		return FUNC_RESULT_SUCCESS;
	}
	return FUNC_RESULT_FAILED;
}
int hashIterator(hashTable_T* table, hashIterator_T* iterator){
	if(NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == iterator)
		return FUNC_RESULT_FAILED_ARGUMENT;
	iterator->HashValue = 0;
	iterator->Compare = false;
	iterator->Key = NULL;
	iterator->KeySize = 0;
	iterator->Item = table->Last;
	iterator->EqualFunction = table->EqualFunction;
	iterator->NextItem = iterator->Item!=NULL ? iterator->Item->ListPrev : NULL;
	return FUNC_RESULT_SUCCESS;
}
bool hashIteratorIsLast(hashIterator_T *item){
	if(NULL == item)
		return false;
	return (NULL == item->Item);
}
int hashIteratorNext(hashIterator_T* item){
	if(NULL == item)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == item->Item)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(item->Compare) {

		do {
			item->Item = item->NextItem;
			item->NextItem = item->NextItem!=NULL ? item->NextItem->Next : NULL;
		}while(!hashIteratorIsLast(item) &&
				!checkEquality(item->Item, item->HashValue, item->Key, item->KeySize, item->EqualFunction));
		return FUNC_RESULT_SUCCESS;

	}
	else{
		item->Item = item->NextItem;
		item->NextItem = item->NextItem!=NULL ? item->NextItem->ListPrev : NULL;
		return FUNC_RESULT_SUCCESS;
	}
}
void* hashIteratorData(hashIterator_T* item){
	if(NULL == item)
		return NULL;
	if(NULL == item->Item)
		return NULL;
	return item->Item->Data;
}
void* hashIteratorKey(hashIterator_T* item){
	if(NULL == item)
		return NULL;
	if(NULL == item->Item)
		return NULL;
	return item->Item->Key;
}
int hashIteratorFree(hashIterator_T* item){
	if(NULL == item)
		return FUNC_RESULT_FAILED_ARGUMENT;
	item->Item = NULL;
	free(item->Key);
	item->Key = NULL;
	item->KeySize = 0;
	item->Compare = false;
	return FUNC_RESULT_SUCCESS;

}
#endif