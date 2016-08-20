//
// Created by kryvashek on 30.07.16.
//

#ifndef MOARSTACK_MOARMESSAGEID_H
#define MOARSTACK_MOARMESSAGEID_H

#include <stdint.h> 	// uint8_t
#include <stdbool.h>	// bool

#include <moarLibInterface.h> // MoarLayerType_T

#define MESSAGE_ID_RAND_SIZE	2
#define MESSAGE_ID_ORDER_SIZE	2
#define MESSAGE_ID_FULL_SIZE	sizeof( MessageId_T )

#pragma pack(push, 1)

typedef struct {
	MoarLayerType_T	SourceLayer;
	uint8_t			PartOrdered[ MESSAGE_ID_ORDER_SIZE ],
					PartRandom[ MESSAGE_ID_RAND_SIZE ];
} MessageId_T;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

// checks whether given message ids are equal; returns true, if are, false otherwise
extern bool midAreEqual( MessageId_T * one, MessageId_T * two );

// generates new identifier for some packet
extern int midGenerate( MessageId_T * identifier, MoarLayerType_T layer );

#ifdef __cplusplus
}
#endif

#endif //MOARSTACK_MOARMESSAGEID_H
