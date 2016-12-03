//
// Created by kryvashek on 08.11.16.
//

#include <moarRoutingTablesHelper.h>
#include "moarRouting.h"		// RouteAddr_T
#include "moarChannel.h"		// ChannelAddr_T
#include "moarRoutingPrivate.h"

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