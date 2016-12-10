//
// Created by spiralis on 05.11.16.
//

#include <moarRouteFinder.h>
#include <moarRoutingStoredPacket.h>

int produceRouteFinder( RoutingLayer_T * layer, RouteAddr_T * destination, RouteAddr_T * next_hop ) {
	if( NULL == layer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	RouteStoredPacket_T packet = { 0 };

	midGenerate( &packet.InternalId, MoarLayer_Routing );
	rmidGenerate( &packet.MessageId );

	packet.Source = layer->LocalAddress;
	packet.Destination = *destination;
	packet.NextProcessing = 0;
	packet.PackType = RoutePackType_Finder;
	packet.State = StoredPackState_InProcessing;
	packet.TrysLeft = DEFAULT_ROUTE_TRYS;
	packet.NextHop = *next_hop;
	packet.XTL = DEFAULT_XTL_FINDER;
	packet.PayloadSize = sizeof( RouteInitialPayloadFinder_T );
	packet.Payload = malloc( packet.PayloadSize );
	if( NULL == packet.Payload ) return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	RouteInitialPayloadFinder_T * payload = ( RouteInitialPayloadFinder_T * )packet.Payload;
	payload->MaxSize = MaxRouteFinderPacketSize;
	int send_result = sendPacketToChannel( layer, &packet );
	free( payload );

	return send_result;
}

int sendFindersFirst( RoutingLayer_T * layer, RouteAddr_T * dest ) {
	hashIterator_T			iterator = { 0 };
	RoutingNeighborInfo_T	* neInfo;
	int 					result,
							count;

	if( NULL == layer || NULL == dest )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = storageIterator( &( layer->NeighborsStorage ), &iterator );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	count = 0;

	while( !hashIteratorIsLast( &iterator ) ) {
		neInfo = storageIteratorData( &iterator );
		hashIteratorNext( &iterator );
		result = produceRouteFinder( layer, dest, &( neInfo->Address ) );

		if( FUNC_RESULT_SUCCESS == result )
			count++;
	}

	hashIteratorFree( &iterator );

	return ( 0 < count ? FUNC_RESULT_SUCCESS : FUNC_RESULT_FAILED_IO );
}
