//
// Created by kryvashek on 07.11.16.
//

#include <moarRoutingPrivate.h>
#include <moarRouteProbe.h>
#include <moarRoutingStoredPacket.h>
#include <memory.h>
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

RouteAddr_T * getProbePayloadAddress( void * probePayload, RouteProbeDepth_T index ) {
	RoutePayloadProbe_T	* start = ( RoutePayloadProbe_T * )probePayload;

	if( NULL == probePayload || index >= start->DepthCurrent )
		return NULL;

	return ( RouteAddr_T * )( start + 1 ) + index;
}

int produceProbeNext( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket, RouteAddr_T * next, RouteStoredPacket_T * newPacket ) {
	RoutePayloadProbe_T	* newPayload,
						* oldPayload;
	RouteProbeDepth_T	newDepthCurrent;
	int 				result;

	if( NULL == layer || NULL == oldPacket || NULL == next || NULL == newPacket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	oldPayload = ( RoutePayloadProbe_T * )( oldPacket->Payload );
	newDepthCurrent = oldPayload->DepthCurrent + ( RouteProbeDepth_T )1;
	newPacket->PayloadSize = sizeof( RoutePayloadProbe_T ) + newDepthCurrent * sizeof( RouteAddr_T );
	newPacket->Payload = malloc( newPacket->PayloadSize );

	if( NULL == newPacket->Payload )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	result = midGenerate( &( newPacket->InternalId ), MoarLayer_Routing );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = rmidGenerate( &( newPacket->MessageId ) );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = helperRoute2Channel( next, &( newPacket->NextHop ) );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	newPayload = ( RoutePayloadProbe_T * )( newPacket->Payload );
	newPayload->DepthMax = oldPayload->DepthMax;
	newPayload->DepthCurrent = newDepthCurrent;
	memcpy( getProbePayloadAddress( newPayload, 0 ), getProbePayloadAddress( oldPayload, 0 ), oldPayload->DepthCurrent * sizeof( RouteAddr_T ) );
	memcpy( getProbePayloadAddress( newPayload, oldPayload->DepthCurrent ), &( layer->LocalAddress ), sizeof( RouteAddr_T ) );
	newPacket->PackType = RoutePackType_Probe;
	newPacket->Source = oldPacket->Source;
	newPacket->Destination = *next;
	newPacket->NextProcessing = timeGetCurrent();
	newPacket->State = StoredPackState_WaitSent; // packet should be sent immediately, but supposed to be sent from another function
	newPacket->TrysLeft = DEFAULT_ROUTE_TRYS;
	newPacket->XTL = oldPacket->XTL;

	return FUNC_RESULT_SUCCESS;
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
	result = clearStoredPacket( &packet );

	return result;
}

