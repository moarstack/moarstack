//
// Created by kryvashek on 09.07.16.
// for private notations

#ifndef MOARSTACK_MOARCHANNELPRIVATE_H
#define MOARSTACK_MOARCHANNELPRIVATE_H

// type for length of universal address
typedef uint8_t UnIfaceAddrLen_T;

// struct for iface address of any length
typedef struct {
	UnIfaceAddrLen_T	Length;
	uint8_t 			* Value;
} UnIfaceAddr_T;

// metadata of packet moving from channel to interface
typedef struct {
	UnIfaceAddr_T	Bridge;
} ChannelMsgDown_T;

const size_t	CHANNEL_MSG_DOWN_SIZE = sizeof( ChannelMsgDown_T );

#endif //MOARSTACK_MOARCHANNELPRIVATE_H
