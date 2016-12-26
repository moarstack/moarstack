//
// Created by svalov on 8/18/16.
//

#ifndef MOARSTACK_HASHTABLE_H
#define MOARSTACK_HASHTABLE_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#define HASH_ENABLE_ITERATOR

#pragma pack(push, 1)

typedef uint32_t hashVal_T;

// hash function proto
typedef hashVal_T (* hashFunc_T)(void* data, size_t size);
typedef int (* equalFunc_T)(const void* key1, const void* key2, size_t size);
typedef void (* freeFunc_T)(void* data);

typedef struct hashEntry_T hashEntry_T;

// hash stuct
struct hashEntry_T{
	hashVal_T 		HashValue;
	void* 			Key;
	void* 			Data;
	hashEntry_T* 	Next;
#ifdef HASH_ENABLE_ITERATOR
	hashEntry_T*  	ListNext;
	hashEntry_T*	ListPrev;
#endif
};
#ifdef HASH_ENABLE_ITERATOR
// hash iterator
typedef struct{
	hashEntry_T* 	Item;
	hashEntry_T* 	NextItem;
	bool 		 	Compare;
	hashVal_T		HashValue;
	equalFunc_T		EqualFunction;
	void* 			Key;
	size_t  		KeySize;
} hashIterator_T;
#endif
// hash table
typedef struct{
	size_t  		KeySize;
	size_t  		DataSize;
	hashFunc_T		HashFunction;
	equalFunc_T		EqualFunction;
	freeFunc_T 		KeyFreeFunction;
	freeFunc_T 		DataFreeFunction;
	int 			StorageSize;
	hashEntry_T**	Table;
	int				Count;
#ifdef HASH_ENABLE_ITERATOR
	hashEntry_T* 	Last;
#endif
}hashTable_T;

#pragma pack(pop)

__BEGIN_DECLS

extern int hashInit(hashTable_T* table, hashFunc_T function, int storageSize, size_t keySize, size_t dataSize);
extern int hashClear(hashTable_T* table);
extern int hashFree(hashTable_T* table);
extern int hashAdd(hashTable_T* table, void* key, void* data);
extern int hashRemove(hashTable_T* table, void* key);
extern int hashRemoveExact(hashTable_T* table, void* key, void* value);
extern int hashGet(hashTable_T* table, void* key, void* data);
extern void* hashGetPtr(hashTable_T* table, void* key);
extern bool hashContain(hashTable_T* table, void* key);
extern bool hashContainExact(hashTable_T* table, void* key, void* data);
#ifdef HASH_ENABLE_ITERATOR
extern int hashGetFirst(hashTable_T* table, void* key, hashIterator_T* iterator);
extern int hashIterator(hashTable_T* table, hashIterator_T* iterator);
extern int hashIteratorNext(hashIterator_T* item);
extern bool hashIteratorEnded( hashIterator_T * item );
extern void* hashIteratorData(hashIterator_T* item);
extern void* hashIteratorKey(hashIterator_T* item);
extern int hashIteratorFree(hashIterator_T* item);
#endif

__END_DECLS

#endif //MOARSTACK_HASHTABLE_H
