//
// Created by svalov on 7/26/16.
//

#ifndef MOARSTACK_MOARMESSAGEPROCESSING_H
#define MOARSTACK_MOARMESSAGEPROCESSING_H

#include <moarChannelPrivate.h>
#include <moarCommons.h>

// register interface
int processRegisterInterface(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command);
// unregister interface
int processUnregisterInterface(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command);
// processing received message
int processReceiveMessage(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command);
//process interface state
int processInterfaceState(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command);
//process new neighbor
int processNewNeighbor(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command);
//process lost neighbor
int processLostNeighbor(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command);
//process update neighbor
int processUpdateNeighbor(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command);
//process message from interface
int processInterfaceData(ChannelLayer_T* layer, int fd, uint32_t event);

//process send message
int processSendMessage(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command);
//process message from routing
int processRoutingData(ChannelLayer_T* layer, int fd, uint32_t event);

int processQueueEntry(ChannelLayer_T* layer, ChannelMessageEntry_T* entry);

#endif //MOARSTACK_MOARMESSAGEPROCESSING_H
