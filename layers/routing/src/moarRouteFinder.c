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
	packet.XTL = DEFAULT_XTL;
	packet.PayloadSize = sizeof( RouteInitialPayloadFinder_T );
	packet.Payload = malloc( packet.PayloadSize );
	if( NULL == packet.Payload ) return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	RouteInitialPayloadFinder_T * payload = ( RouteInitialPayloadFinder_T * )packet.Payload;
	payload->MaxSize = MaxRouteFinderPacketSize;
	int send_result = sendPacketToChannel( layer, &packet );
	free( payload );

	return send_result;
}
