//
// Created by svalov on 10/4/16.
//

#ifndef MOARSTACK_MOARROUTINGSTOREDPACKETFUNC_H
#define MOARSTACK_MOARROUTINGSTOREDPACKETFUNC_H

#include <moarRoutingStoredPacket.h> 

int clearStoredPacket(RouteStoredPacket_T* packet);
int disposeStoredPacket(RouteStoredPacket_T** packet);

int prepareReceivedPacket(RouteStoredPacket_T* packet, ChannelReceiveMetadata_T* metadata, void* data, PayloadSize_T dataSize);
int prepareSentPacket( RouteStoredPacket_T * packet, PresentSendMetadata_T * metadata, void * data, PayloadSize_T dataSize );
int sendPacketToChannel(RoutingLayer_T* layer, RouteStoredPacket_T* packet);
int sendPacketToPresentation( RoutingLayer_T * layer, RouteStoredPacket_T * packet );


#endif //MOARSTACK_MOARROUTINGSTOREDPACKETFUNC_H
