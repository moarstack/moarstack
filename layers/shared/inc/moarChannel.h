//
// Created by kryvashek on 05.07.16.
// for public notations

#ifndef MOARSTACK_MOARCHANNEL_H
#define MOARSTACK_MOARCHANNEL_H

#include <stdint.h>

#define CHANNEL_ADDR_SIZE	8

#pragma pack(push, 1)

typedef struct{
	uint8_t			Address[ CHANNEL_ADDR_SIZE ];
} ChannelAddr_T;
typedef uint16_t	ChannelDataSize_T;

typedef struct{
	ChannelAddr_T 		From;
	ChannelDataSize_T 	PayloadSize;
}ChannelLayerHeader_T;

#define CHANNEL_LAYER_HEADER_SIZE sizeof(ChannelLayerHeader_T)

#pragma pack(pop)

#endif //MOARSTACK_MOARCHANNEL_H
