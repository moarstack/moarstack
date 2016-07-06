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


#define STACK_SIZE (1024*1024)

int createThread(MoarLibrary_T *lib, void *arg){
    int res = pthread_create(&(lib->Thread),NULL, lib->LayerEntryPointFunction, arg);
//    char *stack;
//    char *stackTop;
//    stack = (char*)malloc(STACK_SIZE);
//    stackTop = stack + STACK_SIZE;
//    int pid =  clone(lib->LayerEntryPointFunction, stackTop, CLONE_FS | CLONE_IO | CLONE_VM | CLONE_SIGHAND, arg);
    //lib->LayerEntryPointFunction(NULL);
    if(res)
        return THREAD_CREATE_FAILED;
    return THREAD_CREATE_OK;
}
int exitThread(MoarLibrary_T *lib){
    int res = pthread_cancel(lib->Thread);
    if(res)
        return THREAD_EXIT_FAILED;
    return THREAD_EXIT_OK;
}