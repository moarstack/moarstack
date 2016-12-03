//
// Created by kryvashek on 08.11.16.
//

#include <moarRoutingTablesHelper.h>
#include <moarRoutingPrivate.h>
#include <moarRouteTable.h>
#include <priorityQueue.h>
#include <moarRoutingNeighborsStorage.h>
#include <float.h>
#include <hashTable.h>
#include <memory.h>
#include "moarRouting.h"		// RouteAddr_T
#include "moarChannel.h"		// ChannelAddr_T
#include "moarRoutingPrivate.h"

typedef struct {
	RouteAddr_T		Relay;
	RouteChance_T	Chance;
	bool			Found;
} AimInfo_T;				// todo think about moving into header

int helperFindRelay( RoutingLayer_T * layer, RouteAddr_T * dest, ChannelAddr_T * relay ) {
	RouteDataRecord_T		* row;
	RoutingNeighborInfo_T	neinfo;
	int 					result;

	if( NULL == layer || NULL == dest || NULL == relay )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = storageGet( &( layer->NeighborsStorage ), dest, &neinfo );

	if( FUNC_RESULT_SUCCESS == result )
		result = helperRoute2Channel( dest, relay );
	else {
		row = RouteTableGetRelayBest( &( layer->RouteTable ), *dest );

		if( NULL == row )
			return FUNC_RESULT_FAILED;

		result = helperRoute2Channel( &( row->Relay ), relay );
	}

	return result;
}

int helperUpdateRouteAddrChain( RoutingLayer_T * layer, RouteAddr_T * list, size_t count, bool before ) {
	const ssize_t	step = ( before ? -1 : 1 );
	size_t			left;
	int 			result;

	for( left = --count; left > 0; left--, list += step ) {
		result = helperUpdateRoute( layer, list + step, list );

		if( FUNC_RESULT_SUCCESS == result )
			count--;
	}

	return ( 0 == count ? FUNC_RESULT_SUCCESS : FUNC_RESULT_FAILED );
}

int helperUpdateRouteAddrChainBefore( RoutingLayer_T * layer, RouteAddr_T * list, size_t count ) {
	if( NULL == layer || NULL == list || 0 == count )
		return FUNC_RESULT_FAILED_ARGUMENT;

	return helperUpdateRouteAddrChain( layer, list, count, true );
}

int helperUpdateRouteAddrChainAfter( RoutingLayer_T * layer, RouteAddr_T * list, size_t count ) {
	if( NULL == layer || NULL == list || 0 == count )
		return FUNC_RESULT_FAILED_ARGUMENT;

	return helperUpdateRouteAddrChain( layer, list, count, false );
}

int routeChancesCompare( void * one, void * two, size_t size ) {
	if( *( RouteChance_T * )one > *( RouteChance_T * )two )
		return 1;
	else if( *( RouteChance_T * )one == *( RouteChance_T * )two )
		return 0;
	else
		return -1;
}

int helperSolveRoutes( RoutingLayer_T * layer ) {
	hashTable_T				aims;
	RouteDataRecord_T		* recEntry,
							current,
							child;
	RouteChance_T			newChance;
	RouteAddr_T				* dest;
	AimInfo_T				* aimEntry,
							aim;
	PriorityQueue_T			order;
	hashIterator_T			iterator;
	RoutingNeighborInfo_T	* neInfo;
	int 					result;

	hashInit( &aims, hashRoutingAddress, layer->RouteTable.Count, sizeof( RouteAddr_T ), sizeof( RouteDataRecord_T ) );

	recEntry = RouteTableRowFirst( &( layer->RouteTable ) );
	aim.Chance = 0;
	aim.Found = false;
	memset( &( aim.Relay ), 0, sizeof( aim.Relay ) );

	while( NULL != recEntry ) {
		if( !hashContain( &aims, &( recEntry->Relay ) ) )
			hashAdd( &aims, &( recEntry->Relay ), &aim );

		if( !hashContain( &aims, &( recEntry->Dest ) ) )
			hashAdd( &aims, &( recEntry->Dest ), &aim );

		recEntry = RouteTableRowNext( &( layer->RouteTable ), recEntry );
	}

	pqInit( &order, layer->RouteTable.Count, routeChancesCompare, sizeof( RouteChance_T ), sizeof( RouteDataRecord_T ) );
	result = hashIterator( &( layer->NeighborsStorage.Storage ), &iterator );

	while( FUNC_RESULT_SUCCESS == result && !hashIteratorIsLast( &iterator ) ) {
		neInfo = hashIteratorData( &iterator );
		current.Dest = neInfo->Address;
		current.Relay = neInfo->Address;
		current.P = layer->RouteTable.Settings->RouteDefaultMetric;
		pqEnqueue( &order, &( current.P ), &current );
		result = hashIteratorNext( &iterator );
	}

	while( FUNC_RESULT_SUCCESS == result && 0 < order.Count ) {
		result = pqTop( &order, &current );

		if( FUNC_RESULT_SUCCESS != result )
			continue;

		aimEntry = hashGetPtr( &aims, &( current.Dest ) );

		if( NULL == aimEntry || aimEntry->Found )
			continue;

		aimEntry->Found = true;
		aimEntry->Relay = current.Relay;
		aimEntry->Chance = current.P;

		recEntry = RouteTableRowFirst( &( layer->RouteTable ) );

		while( NULL != recEntry ) {
			if( routeAddrEqualPtr( &( recEntry->Relay ), &( current.Dest ) ) ) {	// if current is a relay for some node
				aimEntry = hashGetPtr( &aims, &( recEntry->Dest ) );				// find new dest in aims
				newChance = ( current.P < recEntry->P ? current.P : recEntry->P );	// new metric calculating: the minimum chance

				if( NULL != aimEntry && !aimEntry->Found && aimEntry->Chance < newChance ) {
					child.Dest = current.Dest;
					child.Relay = current.Relay;
					child.P = newChance;
					pqEnqueue( &order, &( child.P ), &child );
				}
			}

			recEntry = RouteTableRowNext( &( layer->RouteTable ), recEntry );
		}
	}

	pqDeinit( &order );
	hashIterator( &aims, &iterator );
	result = RouteTableClear( &( layer->RouteTableSolved ) );

	while( FUNC_RESULT_SUCCESS == result && !hashIteratorIsLast( &iterator ) ) {
		aimEntry = hashIteratorData( &iterator );
		dest = hashIteratorKey( &iterator );
		RouteTableAdd( &( layer->RouteTableSolved ), aimEntry->Relay, *dest );
		result = hashIteratorNext( &iterator );
	}

	hashFree( &aims );

	return result;
}
