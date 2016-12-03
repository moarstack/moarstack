//
// Created by kryvashek on 07.11.16.
//

#include <moarRoutingPrivate.h>
#include <moarRouteProbe.h>
#include <moarRoutingStoredPacketFunc.h>
#include <memory.h>
#include <moarRoutingTablesHelper.h>
#include <moarNeIterRoutine.h>
#include <moarRoutingStoredPacket.h>

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
	CHECK_RESULT( result );

	result = rmidGenerate( &( packet->MessageId ) );
	CHECK_RESULT( result );

	result = helperRoute2Channel( next, &( packet->NextHop ) );
	CHECK_RESULT( result );

	payload = ( RoutePayloadProbe_T * )( packet->Payload );
	payload->DepthCurrent = 0;
	payload->DepthMax = DEFAULT_PROBE_DEPTH;
	packet->PackType = RoutePackType_Probe;
	packet->Source = layer->LocalAddress;
	packet->Destination = *next;
	packet->NextProcessing = timeGetCurrent();
	packet->State = StoredPackState_WaitSent; // packet should be sent immediately, but supposed to be sent from another function
	packet->TrysLeft = DEFAULT_ROUTE_TRYS;
	packet->XTL = DEFAULT_XTL_PROBE;

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
	CHECK_RESULT( result );

	result = rmidGenerate( &( newPacket->MessageId ) );
	CHECK_RESULT( result );

	result = helperRoute2Channel( next, &( newPacket->NextHop ) );
	CHECK_RESULT( result );

	newPayload = ( RoutePayloadProbe_T * )( newPacket->Payload );
	newPayload->DepthMax = oldPayload->DepthMax;
	newPayload->DepthCurrent = newDepthCurrent;
	memcpy( getProbePayloadAddress( newPayload, 0 ),
            getProbePayloadAddress( oldPayload, 0 ),
            oldPayload->DepthCurrent * sizeof( RouteAddr_T ) );
	memcpy( getProbePayloadAddress( newPayload,
                                    oldPayload->DepthCurrent ),
            &( layer->LocalAddress ),
            sizeof( RouteAddr_T ) );
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
	RouteStoredPacket_T	packet = { 0 };
	RouteAddr_T			address;
	int 				result;

	if( NULL == layer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = neIterFindRandNotNull( &( layer->NeighborsStorage ), &address );
	CHECK_RESULT( result );

	result = produceProbeFirst( layer, &address, &packet );

	if( FUNC_RESULT_SUCCESS != result ) {
		clearStoredPacket( &packet );
		return result;
	}

	result = sendPacketToChannel( layer, &packet );

	if( FUNC_RESULT_SUCCESS != result ) {
		clearStoredPacket( &packet );
		return result;
	}

	layer->NextProbeSentTime = timeGetCurrent();
	result = clearStoredPacket( &packet );

	return result;
}

int sendProbeNext( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket ) {
	RouteStoredPacket_T	newPacket = { 0 };
	RoutePayloadProbe_T	* payload;
	RouteAddrSeekList_T	rasl;
	RouteAddr_T			address;
	int 				result;

	if( NULL == layer || NULL == oldPacket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	payload = ( RoutePayloadProbe_T * )( oldPacket->Payload );
	result = raslInit( &rasl, payload->DepthCurrent + 1, raslCompareDefault );
	CHECK_RESULT( result );

	result = raslSet( &rasl, 0, &( oldPacket->Source ), 1 );
	CHECK_RESULT( result );

	result = raslSet( &rasl, 1, getProbePayloadAddress( payload, 0 ), payload->DepthCurrent );
	CHECK_RESULT( result );

	result = raslSort( &rasl );
	CHECK_RESULT( result );

	result = neIterFindRandNotNullOrUsed( &( layer->NeighborsStorage ), &rasl, &address );

	if( FUNC_RESULT_SUCCESS != result ) {
		result = raslDeinit( &rasl );
		CHECK_RESULT( result );

		result = neIterFindRandNotNull( &( layer->NeighborsStorage ), &address );
		CHECK_RESULT( result );
	}

	result = produceProbeNext( layer, oldPacket, &address, &newPacket );

	if( FUNC_RESULT_SUCCESS != result ) {
		raslDeinit( &rasl );
		clearStoredPacket( &newPacket );
		return result;
	}

	result = sendPacketToChannel( layer, &newPacket );

	if( FUNC_RESULT_SUCCESS != result ) {
		raslDeinit( &rasl );
		clearStoredPacket( &newPacket );
		return result;
	}

	layer->NextProbeSentTime = timeGetCurrent();
	result = clearStoredPacket( &newPacket );

	return result;
}


int processProbePacket( RoutingLayer_T * layer, RouteStoredPacket_T * packet ){

    FUNC_RESULT result;

    if (NULL == layer || NULL == packet){
        return FUNC_RESULT_FAILED_ARGUMENT;
    }

    RoutePayloadProbe_T* payload = packet->Payload;

    if(NULL == payload){
        return FUNC_RESULT_FAILED;
    }

    RouteAddr_T* relay = getProbePayloadAddress(payload,payload->DepthCurrent - 1);
    if(NULL == relay){
        return FUNC_RESULT_FAILED;
    }

    for (int i = 0; i < payload->DepthCurrent; ++i) {
        RouteAddr_T* addr = getProbePayloadAddress(payload,i);
        if(NULL == addr){
            return FUNC_RESULT_FAILED;
        }
        result = (FUNC_RESULT)helperUpdateRoute(layer, addr, relay);
        if (FUNC_RESULT_SUCCESS != result){
            return result;
        }
    }

    return FUNC_RESULT_SUCCESS;
}
