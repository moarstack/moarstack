//
// Created by svalov on 12/15/16.
//

#include "moarServiceConStore.h"
#include <funcResults.h>
#include <moarServiceConStore.h>
#include <hashTable.h>

hashVal_T hashAppId(void *data, size_t size){
	return hashInt32(data,size);
}
void freeStoredAppConnection(void* ptr){
	if(ptr!=NULL)
		free(*(AppConection_T**)ptr);
	free(ptr);
}

int csInit(AppConnectionStorage_T* storage){
	if(NULL == storage)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	int res = hashInit(&storage->appIdTable, hashAppId, TABLE_SIZE, sizeof(AppId_T), sizeof(AppConection_T*));
	CHECK_RESULT(res);
	res = hashInit(&storage->fdTable, hashInt32, TABLE_SIZE, sizeof(uint32_t), sizeof(AppConection_T*));
	storage->fdTable.DataFreeFunction = freeStoredAppConnection;
	return res;
}
int csDeinit(AppConnectionStorage_T* storage){
	if(NULL == storage)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	int res = hashClear(&storage->appIdTable);
	CHECK_RESULT(res);
	res = hashClear(&storage->fdTable);
	CHECK_RESULT(res);
	res = hashFree(&storage->appIdTable);
	CHECK_RESULT(res);
	res = hashFree(&storage->fdTable);
	return res;
}
int csAdd(AppConnectionStorage_T* storage, AppConection_T* connection){
	if(NULL == storage || NULL == connection)
		return FUNC_RESULT_FAILED_ARGUMENT;

	AppConection_T* alloc = malloc(sizeof(AppConection_T));
	if(NULL == alloc)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	memcpy(alloc, connection, sizeof(AppConection_T));

	int res = hashAdd(&storage->fdTable, &alloc->fd, &alloc);
	if(FUNC_RESULT_SUCCESS != res){
		free(alloc);
		return res;
	}
	res = hashAdd(&storage->appIdTable, &alloc->AppId, &alloc);
	if(FUNC_RESULT_SUCCESS != res){
		hashRemove(&storage->fdTable, &alloc->fd);
		free(alloc);
		return res;
	}
	return res;
}
int csRemove(AppConnectionStorage_T* storage, AppConection_T* connection){
	if(NULL == storage || NULL == connection)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = hashRemove(&storage->fdTable, &connection->fd);
	CHECK_RESULT(res);
	res = hashRemove(&storage->appIdTable, &connection->AppId);
	return res;
}
int csGetByFd(AppConnectionStorage_T* storage, AppConection_T* connection, int fd){
	if(NULL == storage || NULL == connection)
		return FUNC_RESULT_FAILED_ARGUMENT;
	AppConection_T* ptr = csGetByFdPtr(storage, fd);
	if(ptr == NULL)
		return FUNC_RESULT_FAILED;
	memcpy(connection,ptr, sizeof(AppConection_T));
	return FUNC_RESULT_SUCCESS;
}
int csGetByAppId(AppConnectionStorage_T* storage, AppConection_T* connection, AppId_T* appId){
	if(NULL == storage || NULL == connection)
		return FUNC_RESULT_FAILED_ARGUMENT;
	AppConection_T* ptr = csGetByAppIdPtr(storage, appId);
	if(ptr == NULL)
		return FUNC_RESULT_FAILED;
	memcpy(connection,ptr, sizeof(AppConection_T));
	return FUNC_RESULT_SUCCESS;
}
AppConection_T* csGetByFdPtr(AppConnectionStorage_T* storage, int fd){
	if(NULL == storage)
		return NULL;
	AppConection_T** ptr = (AppConection_T**)hashGetPtr(&storage->fdTable, &fd);
	if(ptr == NULL)
		return NULL;
	return *ptr;
}
AppConection_T* csGetByAppIdPtr(AppConnectionStorage_T* storage, AppId_T* appId){
	if(NULL == storage)
		return NULL;
	AppConection_T** ptr = (AppConection_T**)hashGetPtr(&storage->appIdTable, appId);
	if(ptr == NULL)
		return NULL;
	return *ptr;
}