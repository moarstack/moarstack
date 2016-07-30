//
// Created by kryvashek on 30.07.16.
//

#ifndef MOARSTACK_MOARMESSAGEID_H
#define MOARSTACK_MOARMESSAGEID_H

#include <moarLibInterface.h> // MoarLayerType_T

#define MESSAGE_ID_FREE_SIZE	7
#define MESSAGE_ID_FULL_SIZE	sizeof( MessageId_T )

#pragma pack(push, 1)

typedef struct {
	MoarLayerType_T	SourceLayer;
	uint8_t			FreePart[ MESSAGE_ID_FREE_SIZE ];
} MessageId_T ;

#pragma pack(pop)

#endif //MOARSTACK_MOARMESSAGEID_H
