//
// Created by kryvashek on 07.11.16.
//

#include <moarRoutingPrivate.h>
#include <moarRouting.h>
#include "moarRouteProbe.h"
#include "moarRoutingTablesHelper.h"

int produceProbeFirst( RoutingLayer_T * layer, RouteAddr_T * next, RouteStoredPacket_T * packet ) {
	RoutePayloadProbe_T	* payload;
	int 				result;

	if( NULL == layer || NULL == next || NULL == packet )
		return FUNC_RESULT_FAILED_ARGUMENT;

	packet->PayloadSize = sizeof( RoutePayloadProbe_T );
	packet->Payload = malloc( packet->PayloadSize );

	if( NULL == packet->Payload )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	result = midGenerate( &( packet->InternalId ), MoarLayer_Routing );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = rmidGenerate( &( packet->MessageId ) );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = helperRoute2Channel( next, &( packet->NextHop ) );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	payload = ( RoutePayloadProbe_T * )( packet->Payload );
	payload->DepthCurrent = 0;
	payload->DepthMax = DEFAULT_PROBE_DEPTH;
	packet->PackType = RoutePackType_Probe;
	packet->Source = layer->LocalAddress;
	packet->Destination = *next;
	packet->NextProcessing = timeGetCurrent();
	packet->State = StoredPackState_WaitSent; // packet should be sent immediately, but supposed to be sent from another function
	packet->TrysLeft = DEFAULT_ROUTE_TRYS;
	packet->XTL = DEFAULT_XTL;

	return FUNC_RESULT_SUCCESS;
}