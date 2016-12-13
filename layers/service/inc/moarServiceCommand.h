//
// Created by svalov on 12/12/16.
//

#ifndef MOARSTACK_MOARSERVICECOMMAND_H
#define MOARSTACK_MOARSERVICECOMMAND_H

#include <moarServicePrivate.h>

int processReceiveCommand(void* layerRef, int fd, LayerCommandStruct_T *command);
int processMsgStateCommand(void* layerRef, int fd, LayerCommandStruct_T *command);

#endif //MOARSTACK_MOARSERVICECOMMAND_H
