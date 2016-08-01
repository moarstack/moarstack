//
// Created by kryvashek on 09.07.16.
// for private notations

#ifndef MOARSTACK_MOARCHANNELPRIVATE_H
#define MOARSTACK_MOARCHANNELPRIVATE_H

#include <moarUnIfaceAddr.h>

// metadata of packet moving from channel to interface
typedef struct {
	UnIfaceAddr_T	Bridge;
} ChannelMsgDown_T;

#define	CHANNEL_MSG_DOWN_SIZE  sizeof( ChannelMsgDown_T )

#endif //MOARSTACK_MOARCHANNELPRIVATE_H
