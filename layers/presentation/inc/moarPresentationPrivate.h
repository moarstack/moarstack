//
// Created by kryvashek on 18.08.16.
//

#ifndef MOARSTACK_MOARPRESENTATIONPRIVATE_H
#define MOARSTACK_MOARPRESENTATIONPRIVATE_H

// type for enumerating parts of one message
typedef uint16_t PresentPartCount_T;

#pragma pack(push, 1)

typedef struct {
	PresentId_T			PrId;
	PresentPartCount_T	Number,
						Amount;
} PresentHeader_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARPRESENTATIONPRIVATE_H
