//
// Created by svalov on 12/21/16.
//

#ifndef MOARSTACK_MOARPRESENTATIONCOMMAND_H
#define MOARSTACK_MOARPRESENTATIONCOMMAND_H

int processSendCommand(void* layerRef, int fd, LayerCommandStruct_T* command);
int processMsgStateCommand(void* layerRef, int fd, LayerCommandStruct_T* command);
int processReceiveCommand(void* layerRef, int fd, LayerCommandStruct_T* command);

#endif //MOARSTACK_MOARPRESENTATIONCOMMAND_H
