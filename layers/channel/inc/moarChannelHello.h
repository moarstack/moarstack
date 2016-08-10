//
// Created by svalov on 8/10/16.
//

#ifndef MOARSTACK_MOARCHANNELHELLO_H
#define MOARSTACK_MOARCHANNELHELLO_H

#include <moarChannelPrivate.h>

typedef struct{

} ChannelHelloMessagePayload_T;

int channelHelloFill(ChannelLayer_T* layer);
int channelHelloUpdateInterface(ChannelLayer_T* layer);
int channelHelloProcess(ChannelLayer_T* layer, PayloadSize_T size, void* data);

#endif //MOARSTACK_MOARCHANNELHELLO_H
