//
// Created by svalov on 7/5/16.
//
#define _GNU_SOURCE
#include <threadManager.h>
#include <pthread.h>
#include <moarLibrary.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <funcResults.h>

int createThread(MoarLibrary_T *lib, void *arg){
    if(NULL == lib)
        return FUNC_RESULT_FAILED_ARGUMENT;
    if(NULL == lib->LayerEntryPointFunction)
        return FUNC_RESULT_FAILED_ARGUMENT;
    //TODO check lib->Thread for valid value
    int res = pthread_create(&(lib->Thread),NULL, lib->LayerEntryPointFunction, arg);
    if(0 != res)
        return FUNC_RESULT_FAILED;
    return FUNC_RESULT_SUCCESS;
}
int exitThread(MoarLibrary_T *lib){
    if(NULL == lib)
        return FUNC_RESULT_FAILED_ARGUMENT;
    //TODO check lib->Thread for valid value
    int res = pthread_cancel(lib->Thread);
    if(0 != res)
        return FUNC_RESULT_FAILED;
    return FUNC_RESULT_SUCCESS;
}