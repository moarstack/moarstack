//
// Created by svalov on 7/29/16.
//

#ifndef MOARSTACK_MOARMESSAGETABLE_H
#define MOARSTACK_MOARMESSAGETABLE_H

int tableFindEntryById(ChannelLayer_T* layer, InterfaceDescriptor_T* iface, MessageId_T* id, ChannelMessageEntry_T* entry);
int tableAddEntry(ChannelLayer_T* layer, InterfaceDescriptor_T* iface, MessageId_T* id, ChannelMessageEntry_T* entry);
int tableDeleteEntry(ChannelLayer_T* layer, InterfaceDescriptor_T* iface, MessageId_T* id);

#endif //MOARSTACK_MOARMESSAGETABLE_H

