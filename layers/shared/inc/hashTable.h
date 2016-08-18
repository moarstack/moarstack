//
// Created by svalov on 8/18/16.
//

#ifndef MOARSTACK_HASHTABLE_H
#define MOARSTACK_HASHTABLE_H

#include <stdlib.h>

// hash function proto
typedef int (* hashFunc_T)(void* data, size_t size);

typedef struct hashEntry_T hashEntry_T;

// hash stuct
struct hashEntry_T{
	int 			HashValue;
	void* 			Key;
	void* 			Data;
	hashEntry_T* 	Next;
};

// hash iterator

// hash table
typedef struct{
	size_t  		KeySize;
	size_t  		DataSize;
	hashFunc_T		HashFunction;
	int 			StorageSize;
	hashEntry_T**	Table;
	int				Count;
}hashTable_T;

#ifdef __cplusplus
extern "C" {
#endif

extern int hashInit(hashTable_T* table, hashFunc_T function, int storageSize, size_t keySize, size_t dataSize);
extern int hashFree(hashTable_T* table);
extern int hashAdd(hashTable_T* table, void* key, void* data);
extern int hashRemove(hashTable_T* table, void* key);
extern int hashGet(hashTable_T* table, void* key, void*);


#ifdef __cplusplus
}
#endif

#endif //MOARSTACK_HASHTABLE_H
