//
// Created by kryvashek on 05.07.16.
// for public notations

#ifndef MOARSTACK_MOARPRESENTATION_H
#define MOARSTACK_MOARPRESENTATION_H

#include <moarPresentId.h>

// type for enumerating parts of one message
typedef uint16_t PresentPartCount_T;

#pragma pack(push, 1)

typedef struct {
	PresentId_T			PrId;
	PresentPartCount_T	Number,
						Amount;
} PresentHeader_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARPRESENTATION_H
