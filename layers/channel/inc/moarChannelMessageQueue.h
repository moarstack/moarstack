//
// Created by svalov on 7/28/16.
//

#ifndef MOARSTACK_MOARCHANNELMESSAGEQUEUE_H
#define MOARSTACK_MOARCHANNELMESSAGEQUEUE_H

#include <moarChannelPrivate.h>

int queueInit(ChannelLayer_T* layer);
int dequeueMessage(ChannelLayer_T* layer, ChannelMessageEntry_T* entry);
int enqueueMessage(ChannelLayer_T* layer, ChannelMessageEntry_T* entry);
int peekMessage(ChannelLayer_T* layer, ChannelMessageEntry_T** entry);

#endif //MOARSTACK_MOARCHANNELMESSAGEQUEUE_H
