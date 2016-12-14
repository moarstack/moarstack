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
		row = RouteTableGetRelayBest( &( layer->RouteTableSolved ), *dest );

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

int findAllAims( RoutingLayer_T * layer, hashTable_T * aims ) {
	AimInfo_T			aim;
	RouteDataRecord_T	* record;
	int 				result;

	if( NULL == layer || NULL == aims )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = hashInit( aims, hashRoutingAddress, layer->RouteTable.Count, sizeof( RouteAddr_T ), sizeof( AimInfo_T ) );
	CHECK_RESULT( result );

	aim.Chance = 0;
	aim.Found = false;
	memset( &( aim.Relay ), 0, sizeof( aim.Relay ) );
	record = RouteTableRowFirst( &( layer->RouteTable ) );

	while( NULL != record ) {
		if( !hashContain( aims, &( record->Relay ) ) ) {
			result = hashAdd( aims, &( record->Relay ), &aim );
			CHECK_RESULT( result );
		}

		if( !hashContain( aims, &( record->Dest ) ) ) {
			result = hashAdd( aims, &( record->Dest ), &aim );
			CHECK_RESULT( result );
		}

		record = RouteTableRowNext( &( layer->RouteTable ), record );
	}

	return FUNC_RESULT_SUCCESS;
}

int prepareQueue( RoutingLayer_T * layer, PriorityQueue_T * order ) {
	hashIterator_T			iterator;
	RoutingNeighborInfo_T	* neInfo;
	RouteDataRecord_T		current;
	int 					result;

	if( NULL == layer || NULL == order )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = hashIterator( &( layer->NeighborsStorage.Storage ), &iterator );
	CHECK_RESULT( result );

	result = pqInit( order, layer->RouteTable.Count, routeChancesCompare, sizeof( RouteChance_T ), sizeof( RouteDataRecord_T ) );
	CHECK_RESULT( result );

	while( !hashIteratorIsLast( &iterator ) ) {
		neInfo = hashIteratorData( &iterator );
		current.Dest = neInfo->Address;
		current.Relay = neInfo->Address;
		current.P = layer->RouteTable.Settings->RouteDefaultMetric; // todo replace with proper when time
		result = pqEnqueue( order, &( current.P ), &current );

		if( FUNC_RESULT_SUCCESS != result ) {
			pqDeinit( order );
			return result;
		}

		result = hashIteratorNext( &iterator );

		if( FUNC_RESULT_SUCCESS != result ) {
			pqDeinit( order );
			return result;
		}
	}

}

int processDijkstra( RoutingLayer_T * layer, hashTable_T * aims ) {
	PriorityQueue_T		order;
	RouteDataRecord_T	current,
						child,
						* record;
	AimInfo_T			* aim;
	RouteChance_T		newChance;
	int 				result;

	if( NULL == layer || NULL == aims )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = prepareQueue( layer, &order );
	CHECK_RESULT( result );

	while( 0 < order.Count ) {
		result = pqTop( &order, &current );

		if( FUNC_RESULT_SUCCESS != result )
			continue;

		aim = hashGetPtr( aims, &( current.Dest ) );

		if( NULL == aim || aim->Found )
			continue;

		aim->Found = true;
		aim->Relay = current.Relay;
		aim->Chance = current.P;

		record = RouteTableRowFirst( &( layer->RouteTable ) );

		while( NULL != record ) {
			if( routeAddrEqualPtr( &( record->Relay ), &( current.Dest ) ) ) {	// if current is a relay for some node
				aim = hashGetPtr( aims, &( record->Dest ) );					// find new dest in aims
				newChance = ( current.P < record->P ? current.P : record->P );	// new metric calculating: the minimum chance

				if( NULL != aim && !aim->Found && aim->Chance < newChance ) {
					child.Dest = current.Dest;
					child.Relay = current.Relay;
					child.P = newChance;
					result = pqEnqueue( &order, &( child.P ), &child );

					if( FUNC_RESULT_SUCCESS != result ) {
						pqDeinit( &order );
						return result;
					}
				}
			}

			record = RouteTableRowNext( &( layer->RouteTable ), record );
		}
	}

	result = pqDeinit( &order );

	return result;
}

int saveSolvedRoutes( RoutingLayer_T * layer, hashTable_T * aims ) {
	RouteAddr_T		* destination;
	AimInfo_T		* aim;
	hashIterator_T	iterator;
	int				result;

	if( NULL == layer || NULL == aims )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = hashIterator( aims, &iterator );
	CHECK_RESULT( result );

	result = RouteTableClear( &( layer->RouteTableSolved ) );
	CHECK_RESULT( result );

	while( !hashIteratorIsLast( &iterator ) ) {
		aim = hashIteratorData( &iterator );
		destination = hashIteratorKey( &iterator );
		result = RouteTableAdd( &( layer->RouteTableSolved ), aim->Relay, *destination );
		CHECK_RESULT( result );

		result = hashIteratorNext( &iterator );
		CHECK_RESULT( result );
	}

}

int helperSolveRoutes( RoutingLayer_T * layer ) {
	hashTable_T				aims;
	int 					result;

	result = findAllAims( layer, &aims );
	CHECK_RESULT( result );

	result = processDijkstra( layer, &aims );
	CHECK_RESULT( result );

	result = saveSolvedRoutes( layer, &aims );
	CHECK_RESULT( result );

	result = hashFree( &aims );
	return result;
}
