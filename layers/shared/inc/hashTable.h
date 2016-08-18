//
// Created by svalov on 8/18/16.
//

#ifndef MOARSTACK_HASHTABLE_H
#define MOARSTACK_HASHTABLE_H

// hash function proto
typedef int (* hashFunc_T)(void* data, size_t size);
// hash iterator
typedef  struct{
	int 	HashValue;
	void* 	Key;
	void* 	Data;
}hashEntry_T;

// hash table
typedef struct{
	size_t  		KeySize;
	size_t  		DataSize;
	hashFunc_T		HashFunction;
	int 			StorageSize;
	void**			Table; // TODO move type to list
}hashTable_T;

#ifdef __cplusplus
extern "C" {
#endif

extern int hashInit(hashTable_T* table, hashFunc_T function, int storageSize, size_t keySize, size_t dataSize);
extern int hashFree(hashTable_T* table);
extern int hashAdd(hashTable_T* table, void* key, void* data);
extern int hashRemove(hashTable_T* table, void* key);
extern void* hashGet(hashTable_T* table, void* key);


#ifdef __cplusplus
}
#endif

#endif //MOARSTACK_HASHTABLE_H
