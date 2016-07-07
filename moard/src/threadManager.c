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
    int res = pthread_create(&(lib->Thread),NULL, lib->LayerEntryPointFunction, arg);
    if(0 != res)
        return THREAD_CREATE_FAILED;
    return THREAD_CREATE_OK;
}
int exitThread(MoarLibrary_T *lib){
    int res = pthread_cancel(lib->Thread);
    if(0 != res)
        return THREAD_EXIT_FAILED;
    return THREAD_EXIT_OK;
}