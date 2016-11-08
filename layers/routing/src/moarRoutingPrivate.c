//
// Created by kryvashek on 08.11.16.
//

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