//
// Created by kryvashek on 18.08.16.
//

#ifndef MOARSTACK_MOARPRESENTID_H
#define MOARSTACK_MOARPRESENTID_H

#include <stdbool.h>	// bool
#include <moarMessageId.h>
#include <moarSettingsBinder.h>
#include <moarInterface.h>

#define PRESENT_ID_SIZE	sizeof( MessageId_T )

#pragma pack(push, 1)

typedef struct {
	uint8_t Value[ PRESENT_ID_SIZE ];
} PresentId_T;

#pragma pack(pop)

__BEGIN_DECLS

// checks whether given message ids are equal; returns true, if are, false otherwise
extern bool pridAreEqual( PresentId_T * one, PresentId_T * two );

// generates new identifier for some packet
extern PresentId_T pridGenerate( void );

// converts internal message id into presentation id
extern PresentId_T pridConvert( MessageId_T internalId );

__END_DECLS

#endif //MOARSTACK_MOARPRESENTID_H
