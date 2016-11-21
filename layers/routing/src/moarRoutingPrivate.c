//
// Created by kryvashek on 08.11.16.
//

#include <moarRoutingTablesHelper.h>
#include <moarRoutingPrivate.h>
#include <moarRouteTable.h>
#include <queue.h>
#include <moarRoutingNeighborsStorage.h>
#include <float.h>
#include <hashTable.h>
#include <memory.h>
#include "moarRouting.h"		// RouteAddr_T
#include "moarChannel.h"		// ChannelAddr_T
#include "moarRoutingPrivate.h"

typedef struct {
	float		weight;
	RouteAddr_T	parent,
				root;
} AimInfo_T;			// todo think about moving into header

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
	int 			result;

	for( count--; count > 0; list += step ) {
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

int helperSolveRoutes( RoutingLayer_T * layer ) {
	hashTable_T				aims;
	RouteDataRecord_T		* row;
	AimInfo_T				tempInfo = { .weight = FLT_MAX },
							* current;
	Queue_T					order;
	hashIterator_T			neIter;
	RoutingNeighborInfo_T	* neInfo;
	bool					* used;

	hashInit( &aims, hashRoutingAddress, layer->RouteTable.Count, sizeof( RouteAddr_T ), sizeof( AimInfo_T ) );

	row = RouteTableRowFirst( &( layer->RouteTable ) );

	while( NULL != row ) {
		if( !hashContain( &aims, &( row->Relay ) ) )
			hashAdd( &aims, &( row->Relay ), &tempInfo );

		if( !hashContain( &aims, &( row->Dest ) ) )
			hashAdd( &aims, &( row->Dest ), &tempInfo );

		row = RouteTableRowNext( &( layer->RouteTable ), row );
	}

	used = calloc( aims.Count, sizeof( bool ) );
	queueInit( &order, sizeof( RouteAddr_T ) );
	hashIterator( &( layer->NeighborsStorage ), &neIter );

	while( !hashIteratorIsLast( &neIter ) ) {
		neInfo = hashIteratorData( &neIter );
		queueEnqueue( &order, &( neInfo->Address ) );
		current = hashGetPtr( &aims, &( neInfo->Address ) );
		current->weight = 0; // TODO add metric to this neighbor

		while( 0 < order.Count ) {

		}

		hashIteratorNext( &neIter );

		if( !hashIteratorIsLast( &neIter ) ) {
			queueClear( &order );
			memset( used, 0, aims.Count * sizeof( bool ) );
		}
	}


	queueDeinit( &order );
	hashFree( &aims );
}