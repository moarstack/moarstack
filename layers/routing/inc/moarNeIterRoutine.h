//
// Created by kryvashek on 09.11.16.
//

#ifndef MOARSTACK_MOARNEITERROUTINE_H
#define MOARSTACK_MOARNEITERROUTINE_H

#include <moarRoutingPrivate.h>
#include "moarRoutingNeighborsStorage.h"

typedef int ( * RouteAddrCompare_F)( const void * one, const void * two );

typedef struct {
	RouteAddr_T			* List;
	size_t 				Count;
	RouteAddrCompare_F	CompareFunc;
} RouteAddrSeekList_T;

extern int raslCompareDefault( const void * one, const void * two );
extern int raslInit( RouteAddrSeekList_T * rasl, size_t count, RouteAddrCompare_F compareFunc );
extern int raslSet( RouteAddrSeekList_T * rasl, size_t start, RouteAddr_T * source, size_t count );
extern int raslSort( RouteAddrSeekList_T * rasl );
extern RouteAddr_T * raslSeek( RouteAddrSeekList_T * rasl, RouteAddr_T * needle );
extern int raslDeinit( RouteAddrSeekList_T * rasl );

extern int neIterFindRandNotNUll( RoutingNeighborsStorage_T * storage, RouteAddr_T * address );
extern int neIterFindRandNotNUllOrUsed( RoutingNeighborsStorage_T * storage, RouteAddrSeekList_T * rasl, RouteAddr_T * address );

#endif //MOARSTACK_MOARNEITERROUTINE_H
