//
// Created by svalov on 7/27/16.
//

#ifndef MOARSTACK_MOARCHANNELNEIGHBORS_H
#define MOARSTACK_MOARCHANNELNEIGHBORS_H

#endif //MOARSTACK_MOARCHANNELNEIGHBORS_H

int neighborsInit(ChannelLayer_T* layer);
int neighborsDeinit(ChannelLayer_T* layer);
ChannelNeighbor_T* neighborFind(ChannelLayer_T* layer, ChannelAddr_T* address);
RemoteInterface_T* neighborIfaceFind(ChannelNeighbor_T* neighbor);
RemoteInterface_T* neighborIfaceFindAddr(ChannelNeighbor_T* neighbor, UnIfaceAddr_T* remoteAddress);
int neighborAdd(ChannelLayer_T* layer, ChannelAddr_T* address, UnIfaceAddr_T* remoteAddress, int localSocket);
int neighborRemove(ChannelLayer_T* layer, UnIfaceAddr_T* remoteAddress); //todo change signature
int neighborsRemoveAssociated(ChannelLayer_T* layer, int localSocket);
//non resolved processing
int neighborNonResAdd(ChannelLayer_T* layer, UnIfaceAddr_T* remoteAddress, int localSocket);
int neighborNonResProcess(ChannelLayer_T* layer);
int neighborNonResRemove(ChannelLayer_T* layer, UnIfaceAddr_T* address);
NonResolvedNeighbor_T* neighborNonResFind(ChannelLayer_T* layer, UnIfaceAddr_T* address);
