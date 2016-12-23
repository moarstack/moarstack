//
// Created by kryvashek on 12.11.16.
//

#include <moarRouting.h>
#include <moarRoutingTablesHelper.h>
#include <moarRouteFinder.h>
#include "moarRouteFinderAck.h"

int processPayloadFack( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
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
		CHECK_RESULT( result );
	}

	result = helperSolveRoutes( layer ); // it is just an example of usage

	return result;
}

int produceFack( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket, RouteStoredPacket_T * newPacket ) {
	void	* oldPayload;
	int 	result;

	if( NULL == layer || NULL == oldPacket || NULL == oldPacket->Payload ||
			sizeof( RouteInitialPayloadFinder_T ) > oldPacket->PayloadSize || NULL == newPacket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	newPacket->PayloadSize = oldPacket->PayloadSize - sizeof( RouteInitialPayloadFinder_T );
	newPacket->Payload = malloc( newPacket->PayloadSize );

	if( NULL == newPacket->Payload )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	result = midGenerate( &( newPacket->InternalId ), MoarLayer_Routing );
	CHECK_RESULT( result );

	result = rmidGenerate( &( newPacket->MessageId ) );
	CHECK_RESULT( result );

	newPacket->NextHop = oldPacket->LastHop;
	oldPayload = ( RouteInitialPayloadFinder_T * )( oldPacket->Payload ) + 1;
	memcpy( newPacket->Payload, oldPayload, newPacket->PayloadSize );
	newPacket->PackType = RoutePackType_FinderAck;
	newPacket->Source = layer->LocalAddress;
	newPacket->Destination = oldPacket->Source;
	newPacket->NextProcessing = timeGetCurrent();
	newPacket->State = StoredPackState_InProcessing;
	newPacket->TrysLeft = DEFAULT_ROUTE_TRYS;
	newPacket->XTL = DEFAULT_XTL_FACK;

	return FUNC_RESULT_SUCCESS;
}

int sendFack( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket ) {
	RouteStoredPacket_T	newPacket = { 0 };
	int 				result;

	result = produceFack( layer, oldPacket, &newPacket ); // create finder ack

	if( FUNC_RESULT_SUCCESS != result ) {
		clearStoredPacket( &newPacket );
		return result;
	}

	result = psAdd( &( layer->PacketStorage ), &newPacket ); // add finder ack with procesing state

	if( FUNC_RESULT_SUCCESS != result )
		clearStoredPacket( &newPacket );

	return result;
}