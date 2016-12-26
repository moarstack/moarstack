//
// Created by svalov on 8/10/16.
//

#ifndef MOARSTACK_MOARCHANNELHELLO_H
#define MOARSTACK_MOARCHANNELHELLO_H

#include <moarChannelPrivate.h>

#pragma pack(push, 1)

typedef struct{

} ChannelHelloMessagePayload_T;

#pragma pack(pop)

int channelHelloFill(ChannelLayer_T* layer);
int channelHelloUpdateInterface(ChannelLayer_T* layer);
int channelHelloProcess(ChannelLayer_T* layer, PayloadSize_T size, void* data);
int channelHelloSendToNeighbor(ChannelLayer_T* layer, UnIfaceAddr_T* address, InterfaceDescriptor_T* bridge);

#endif //MOARSTACK_MOARCHANNELHELLO_H
