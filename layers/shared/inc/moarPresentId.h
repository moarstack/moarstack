//
// Created by kryvashek on 18.08.16.
//

#ifndef MOARSTACK_MOARPRESENTID_H
#define MOARSTACK_MOARPRESENTID_H

#include <stdint.h> 	// uint16_t
#include <stdbool.h>	// bool

#define PRESENT_ID_ORDER_SIZE	sizeof(PresentIdOrder_T)
#define PRESENT_ID_RAND_SIZE	sizeof(PresentIdRand_T)
#define PRESENT_ID_FULL_SIZE	(PRESENT_ID_ORDER_SIZE+PRESENT_ID_RAND_SIZE)

typedef uint16_t	PresentIdOrder_T;
typedef uint16_t	PresentIdRand_T;

#pragma pack(push, 1)

typedef struct {
	PresentIdOrder_T	Order;
	PresentIdRand_T		Rand;
} PresentId_T ;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

// checks whether given message ids are equal; returns true, if are, false otherwise
extern bool prIdAreEqual( PresentId_T * one, PresentId_T * two );

// generates new identifier for some packet
extern PresentId_T prIdGenerate( void );

#ifdef __cplusplus
}
#endif

#endif //MOARSTACK_MOARPRESENTID_H
