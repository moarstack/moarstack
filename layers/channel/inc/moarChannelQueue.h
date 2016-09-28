//
// Created by svalov on 7/28/16.
//

#ifndef MOARSTACK_MOARCHANNELMESSAGEQUEUE_H
#define MOARSTACK_MOARCHANNELMESSAGEQUEUE_H

#include <moarChannelPrivate.h>

int messageQueueInit(ChannelLayer_T* layer);
// if entry is null remove element without copy
int messageDequeue(ChannelLayer_T* layer, ChannelMessageEntry_T* entry);
int messageEnqueue(ChannelLayer_T* layer, ChannelMessageEntry_T* entry);
int messagePeek(ChannelLayer_T* layer, ChannelMessageEntry_T** entry);

#endif //MOARSTACK_MOARCHANNELMESSAGEQUEUE_H
