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
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CANCEL_ENABLE | PTHREAD_CREATE_JOINABLE);

    //TODO check lib->Thread for valid value
    int res = pthread_create(&(lib->Thread), &attrs, lib->LayerEntryPointFunction, arg);
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
    res = pthread_join(lib->Thread, NULL);
	if(0 != res)
		return FUNC_RESULT_FAILED;
    return FUNC_RESULT_SUCCESS;
}