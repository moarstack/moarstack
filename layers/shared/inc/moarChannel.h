//
// Created by kryvashek on 05.07.16.
// for public notations

#ifndef MOARSTACK_MOARCHANNEL_H
#define MOARSTACK_MOARCHANNEL_H

#include <stdint.h>

#define CHANNEL_ADDR_SIZE	8

typedef uint8_t	ChannelAddr_T[ CHANNEL_ADDR_SIZE ];

typedef uint8_t IfacesCount_T;

typedef struct{
	int				UpSocketHandler,
					* DownSocketHandlers;
	IfacesCount_T	IfacesCount;
} MoarChannelStartupParams_T;

#endif //MOARSTACK_MOARCHANNEL_H
