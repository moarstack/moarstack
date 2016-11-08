//
// Created by kryvashek on 07.11.16.
//

#include <moarRoutingPrivate.h>
#include <moarRouteProbe.h>
#include "moarRoutingTablesHelper.h"
#include "moarRoutingStoredPacketFunc.h"

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
	payload->DepthCurrent = 1;
	payload->DepthMax = DEFAULT_PROBE_DEPTH;
	payload->List[ 0 ] = *next;
	packet->PackType = RoutePackType_Probe;
	packet->Source = layer->LocalAddress;
	packet->Destination = *next;
	packet->NextProcessing = timeGetCurrent();
	packet->State = StoredPackState_WaitSent; // packet should be sent immediately, but supposed to be sent from another function
	packet->TrysLeft = DEFAULT_ROUTE_TRYS;
	packet->XTL = DEFAULT_XTL;

	return FUNC_RESULT_SUCCESS;
}

int produceProbeNext( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket, RouteAddr_T * next, RouteStoredPacket_T * newPacket ) {
	if( NULL == layer || NULL == oldPacket || NULL == next || NULL == newPacket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	return FUNC_RESULT_FAILED; // not implemented yet
}

int sendProbeFirst( RoutingLayer_T * layer ) {
	RouteStoredPacket_T		packet;
	RoutingNeighborInfo_T	* neInfo;
	hashIterator_T			iterator = { 0 };
	int 					choosed, index, result;

	if( NULL == layer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	choosed = rand() % layer->NeighborsStorage.Count;

	result = storageIterator( &( layer->NeighborsStorage ), &iterator );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	for( index = 0; FUNC_RESULT_SUCCESS == result && index < choosed; index++ )
		if( !hashIteratorIsLast( &iterator ) )
			result = hashIteratorNext( &iterator );

	neInfo = hashIteratorData( &iterator );

	while( FUNC_RESULT_SUCCESS == result && NULL == neInfo && !hashIteratorIsLast( &iterator ) ) {
		result = hashIteratorNext( &iterator );
		neInfo = hashIteratorData( &iterator );
	}

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = produceProbeFirst( layer, &( neInfo->Address ), &packet );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = sendPacketToChannel( layer, &packet );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	layer->NextProbeSentTime = timeGetCurrent();

	return FUNC_RESULT_SUCCESS;
}