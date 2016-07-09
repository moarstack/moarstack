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


int createThread(MoarLibrary_T *lib, void *arg){
    if(NULL == lib)
        return THREAD_CREATE_FAILED;
    if(NULL == lib->LayerEntryPointFunction)
        return THREAD_CREATE_FAILED;
    //TODO check lib->Thread for valid value
    int res = pthread_create(&(lib->Thread),NULL, lib->LayerEntryPointFunction, arg);
    if(0 != res)
        return THREAD_CREATE_FAILED;
    return THREAD_CREATE_OK;
}
int exitThread(MoarLibrary_T *lib){
    if(NULL == lib)
        return THREAD_EXIT_FAILED;
    //TODO check lib->Thread for valid value
    int res = pthread_cancel(lib->Thread);
    if(0 != res)
        return THREAD_EXIT_FAILED;
    return THREAD_EXIT_OK;
}