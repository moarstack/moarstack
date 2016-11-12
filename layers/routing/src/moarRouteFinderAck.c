//
// Created by kryvashek on 12.11.16.
//

#include <moarRouting.h>
#include <moarRoutingTablesHelper.h>
#include <funcResults.h>
#include "moarRouteFinderAck.h"

int processPayloadFinderAck( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	size_t		position;
	RouteAddr_T	lastHopRouteAddr,
				* list;
	int			result,
				count;

	if( NULL == layer || NULL == packet || NULL == packet->Payload || 0 == packet->PayloadSize )
		return FUNC_RESULT_FAILED_ARGUMENT;

	list = ( RouteAddr_T * )( packet->Payload );
	result = helperChannel2Route( &( packet->LastHop ), &lastHopRouteAddr );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	helperUpdateNeighbor( layer ); // TODO implement fine neighbor updating

	for( position = packet->PayloadSize / sizeof( RouteAddr_T ) - 1; position > 0; position-- )
		if( routeAddrEqualPtr( list + position, &lastHopRouteAddr ) )
			break;

	for( count = 0; position > 0; position-- ) {
		result = helperUpdateRoute( layer, list + position + 1, list + position );

		if( FUNC_RESULT_SUCCESS == result )
			count++;
	}

	return ( 0 < count ? FUNC_RESULT_SUCCESS : FUNC_RESULT_FAILED );
}