//
// Created by svalov on 7/26/16.
//

#ifndef MOARSTACK_MOARINTERFACES_H
#define MOARSTACK_MOARINTERFACES_H

#include <moarChannelPrivate.h>

// functions
int interfaceInit(ChannelLayer_T* layer);
// add interface
int interfaceAdd(ChannelLayer_T* layer, InterfaceDescriptor_T* iface);
// remove interface
int interfaceRemove(ChannelLayer_T* layer, int fd);
// find interface
InterfaceDescriptor_T* interfaceFindBySocket(ChannelLayer_T *layer, int fd);

#endif //MOARSTACK_MOARINTERFACES_H
