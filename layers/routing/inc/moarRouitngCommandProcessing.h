//
// Created by svalov on 8/13/16.
//

#ifndef MOARSTACK_MOARROUITNGCOMMANDPROCESSING_H
#define MOARSTACK_MOARROUITNGCOMMANDPROCESSING_H


int processReceiveCommand(void* layerRef, int fd, LayerCommandStruct_T* command);
int processMessageStateCommand(void* layerRef, int fd, LayerCommandStruct_T* command);
int processNewNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command);
int processLostNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command);
int processUpdateNeighborCommand(void* layerRef, int fd, LayerCommandStruct_T* command);
int processSendCommand(void* layerRef, int fd, LayerCommandStruct_T* command);

#endif //MOARSTACK_MOARROUITNGCOMMANDPROCESSING_H
