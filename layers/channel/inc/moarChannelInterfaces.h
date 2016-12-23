//
// Created by svalov on 7/26/16.
//

#ifndef MOARSTACK_MOARINTERFACES_H
#define MOARSTACK_MOARINTERFACES_H

#include <moarChannelPrivate.h>

// functions
int interfaceInit(ChannelLayer_T* layer);
// add interface
//int interfaceAdd(ChannelLayer_T* layer, InterfaceDescriptor_T* iface);
int interfaceAdd(ChannelLayer_T* layer, UnIfaceAddr_T* address, int socket);
// remove interface
int interfaceRemove(ChannelLayer_T* layer, int fd);
// find interface
InterfaceDescriptor_T* interfaceFind(ChannelLayer_T *layer, int fd);
int interfaceDeinit(ChannelLayer_T* layer);

#endif //MOARSTACK_MOARINTERFACES_H
