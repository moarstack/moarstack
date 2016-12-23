//
// Created by svalov on 7/26/16.
//

#ifndef MOARSTACK_MOARMESSAGEPROCESSING_H
#define MOARSTACK_MOARMESSAGEPROCESSING_H

#include <moarChannelPrivate.h>
#include <moarCommons.h>

// register interface
int processRegisterInterface(void* layerRef, int fd, LayerCommandStruct_T *command);
// unregister interface
int processUnregisterInterface(void* layerRef, int fd, LayerCommandStruct_T *command);
// processing received message
int processReceiveMessage(void* layerRef, int fd, LayerCommandStruct_T *command);
//process interface state
int processInterfaceState(void* layerRef, int fd, LayerCommandStruct_T *command);
//process new neighbor
int processNewNeighbor(void* layerRef, int fd, LayerCommandStruct_T *command);
//process lost neighbor
int processLostNeighbor(void* layerRef, int fd, LayerCommandStruct_T *command);
//process update neighbor
int processUpdateNeighbor(void* layerRef, int fd, LayerCommandStruct_T *command);
//process send message
int processSendMessage(void* layerRef, int fd, LayerCommandStruct_T *command);
//process queue entry
int processQueueEntry(ChannelLayer_T* layer, ChannelMessageEntry_T* entry);
//process all queue
int processQueue(ChannelLayer_T* layer);
#endif //MOARSTACK_MOARMESSAGEPROCESSING_H
