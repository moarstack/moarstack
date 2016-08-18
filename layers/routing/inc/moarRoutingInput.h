//
// Created by svalov on 8/16/16.
//

#ifndef MOARSTACK_MOARROUTINGINPUT_H
#define MOARSTACK_MOARROUTINGINPUT_H

#include <moarChannelRouting.h>
#include <moarRoutingStoredPacket.h>
#include <moarRoutingPresentation.h>

int processInputFromChannel(RouteStoredPacket_T* packet, ChannelReceiveMetadata_T* metadata, void* data, PayloadSize_T dataSize);
//int processInputFromPresentation(RouteStoredPacket_T* packet, PresentationSendMetadata_T* metadata, void* data, PayloadSize_T dataSize);

#endif //MOARSTACK_MOARROUTINGINPUT_H
