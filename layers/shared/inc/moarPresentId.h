//
// Created by kryvashek on 18.08.16.
//

#ifndef MOARSTACK_MOARPRESENTID_H
#define MOARSTACK_MOARPRESENTID_H

#include <stdbool.h>	// bool

#include <moarMessageId.h>

typedef MessageId_T	PresentId_T;

#ifdef __cplusplus
extern "C" {
#endif

// checks whether given message ids are equal; returns true, if are, false otherwise
extern bool pridAreEqual( PresentId_T * one, PresentId_T * two );

// generates new identifier for some packet
extern PresentId_T pridGenerate( void );

#ifdef __cplusplus
}
#endif

#endif //MOARSTACK_MOARPRESENTID_H
