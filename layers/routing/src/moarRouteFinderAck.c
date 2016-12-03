//
// Created by kryvashek on 12.11.16.
//

#include <moarRouting.h>
#include <moarRoutingTablesHelper.h>
#include <funcResults.h>
#include <moarRoutingPrivate.h>
#include <moarRoutingStoredPacket.h>
#include "moarRouteFinderAck.h"

int processPayloadFinderAck( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	size_t		count,
				position;
	RouteAddr_T	* list,
				lastHopRouteAddr;
	int			result;

	if( NULL == layer || NULL == packet || NULL == packet->Payload || 0 == packet->PayloadSize )
		return FUNC_RESULT_FAILED_ARGUMENT;

	list = ( RouteAddr_T * )( packet->Payload );
	count = packet->PayloadSize / sizeof( RouteAddr_T );

	result = helperChannel2Route( &( packet->LastHop ), &lastHopRouteAddr );
	CHECK_RESULT( result );

	if( routeAddrEqualPtr( &( packet->Source ), &lastHopRouteAddr ) )
		position = count;
	else
		for( position = 0; position < count; position++ )
			if( routeAddrEqualPtr( list + position, &lastHopRouteAddr ) )
				break;

	if( 1 < position ) {
		result = helperUpdateRouteAddrChainBefore( layer, list + position - 2, position - 1 );
		CHECK_RESULT( result );

		result = helperUpdateRoute( layer, &( packet->Destination ), list );
		CHECK_RESULT( result );
		}

	if( count > position ) {
		result = helperUpdateRouteAddrChainAfter( layer, list + position, count - position - 1 );
		CHECK_RESULT( result );

		result = helperUpdateRoute( layer, &( packet->Source ), list + count - 1 );
	}

	return result;
}