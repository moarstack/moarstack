//
// Created by svalov on 12/15/16.
//

#ifndef MOARSTACK_MOARSERVICEAPPLICATIONS_H
#define MOARSTACK_MOARSERVICEAPPLICATIONS_H

#include <hashTable.h>
#include <moarCommons.h>
#include <moarApiCommon.h>
#include <moarPresentationService.h>

#define TABLE_SIZE 1024

typedef struct{
	int fd;
	AppId_T AppId;
}AppConection_T;

typedef struct{
	hashTable_T fdTable;
	hashTable_T appIdTable;
	int Count;
}AppConnectionStorage_T;

int csInit(AppConnectionStorage_T* storage);
int csDeinit(AppConnectionStorage_T* storage);
int csAdd(AppConnectionStorage_T* storage, AppConection_T* connection);
int csRemove(AppConnectionStorage_T* storage, AppConection_T* connection);
int csGetByFd(AppConnectionStorage_T* storage, AppConection_T* connection, int fd);
int csGetByAppId(AppConnectionStorage_T* storage, AppConection_T* connection, AppId_T* appId);
AppConection_T* csGetByFdPtr(AppConnectionStorage_T* storage, int fd);
AppConection_T* csGetByAppIdPtr(AppConnectionStorage_T* storage, AppId_T* appId);


#endif //MOARSTACK_MOARSERVICEAPPLICATIONS_H
