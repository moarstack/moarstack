//
// Created by svalov on 8/16/16.
//

#ifndef MOARSTACK_MOARROUINTGMESSAGEID_H
#define MOARSTACK_MOARROUINTGMESSAGEID_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <hashFunc.h>

#define ROUTING_MESSAGE_ID_SIZE	8

#pragma pack(push, 1)

typedef struct {
	uint8_t		Data[ ROUTING_MESSAGE_ID_SIZE ];
} RoutingMessageId_T ;

#pragma pack(pop)

__BEGIN_DECLS

extern hashVal_T rmidHash(void* id, size_t size);
extern bool rmidEqual( RoutingMessageId_T * one, RoutingMessageId_T * two );
extern int rmidGenerate( RoutingMessageId_T * identifier);

__END_DECLS

#endif //MOARSTACK_MOARROUINTGMESSAGEID_H
