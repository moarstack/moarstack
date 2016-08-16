//
// Created by svalov on 8/16/16.
//

#ifndef MOARSTACK_MOARROUINTGMESSAGEID_H
#define MOARSTACK_MOARROUINTGMESSAGEID_H

#include <stdbool.h>
#include <stdint.h>

#define ROUTING_MESSAGE_ID_SIZE	8

#pragma pack(push, 1)

typedef struct {
	uint8_t		Data[ ROUTING_MESSAGE_ID_SIZE ];
} RoutingMessageId_T ;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif //MOARSTACK_MOARROUINTGMESSAGEID_H
