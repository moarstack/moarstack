//
// Created by spiralis on 05.11.16.
//

#include <moarRouteFinder.h>
#include <moarRoutingPrivate.h>
#include <moarRoutingStoredPacket.h>


int produceInitialRouteFinder(RoutingLayer_T *layer, RouteAddr_T *destination, RouteAddr_T *next_hop,
                              RouteStoredPacket_T *ptr) {
    if (NULL == layer|| NULL == destination || NULL == next_hop)
        return FUNC_RESULT_FAILED_ARGUMENT;

    RouteStoredPacket_T *packet = ptr;

    midGenerate(&packet->InternalId, MoarLayer_Routing);
    rmidGenerate(&packet->MessageId);

    packet->Source = layer->LocalAddress;
    packet->Destination = *destination;
    packet->NextProcessing = 0;
    packet->PackType = RoutePackType_Finder;
    packet->State = StoredPackState_InProcessing;
    packet->TrysLeft = DEFAULT_ROUTE_TRYS;
    packet->NextHop = *next_hop;
	packet->XTL = DEFAULT_XTL_FINDER;
    packet->PayloadSize = sizeof(RouteInitialPayloadFinder_T);
    packet->Payload = malloc(packet->PayloadSize);
    if (NULL == packet->Payload) return FUNC_RESULT_FAILED_MEM_ALLOCATION;
    RouteInitialPayloadFinder_T* payload = (RouteInitialPayloadFinder_T*)packet->Payload;
    payload->MaxSize = MaxRouteFinderPacketSize;
    return FUNC_RESULT_SUCCESS;
}

int sendFindersFirst(RoutingLayer_T *layer, RouteAddr_T *dest) {
    hashIterator_T			iterator = { 0 };
    RoutingNeighborInfo_T	* neInfo;
    int 					result,
            count;

    if( NULL == layer || NULL == dest )
        return FUNC_RESULT_FAILED_ARGUMENT;

	if( 0 == layer->NeighborsStorage.Count )
		return FUNC_RESULT_FAILED_NEIGHBORS;

    result = storageIterator( &( layer->NeighborsStorage ), &iterator );

    if( FUNC_RESULT_SUCCESS != result )
        return result;

    count = 0;

    while( !hashIteratorEnded( &iterator ) ) {
        neInfo = storageIteratorData( &iterator );
        hashIteratorNext( &iterator );

        RouteStoredPacket_T packet;
        result = produceInitialRouteFinder(layer, dest, &(neInfo->Address), &packet);
        if(FUNC_RESULT_SUCCESS == result)
        {
            result = sendPacketToChannel(layer, &packet);
            clearStoredPacket(&packet);
        }

        if( FUNC_RESULT_SUCCESS == result )
            count++;
    }

    hashIteratorFree( &iterator );

    return ( 0 < count ? FUNC_RESULT_SUCCESS : FUNC_RESULT_FAILED_IO );
}

////тут мы прикидываем из старого пакета с частью пути делаем новый и перекидываем дальше


// по пейлоуду предыдущего пакета делаем пейлоуд для следующего
int getNextFinderPayload( RoutingLayer_T * layer, void * oldPayload, PayloadSize_T oldSize, void ** newPayload, PayloadSize_T * newSize ) {
	RouteInitialPayloadFinder_T	* nextPayload;
	RouteAddr_T					* newAddress;
	PayloadSize_T				recCount;

	if( NULL == layer || NULL == oldPayload || 0 == oldSize || NULL == newPayload || NULL == newSize )
		return FUNC_RESULT_FAILED_ARGUMENT;

	recCount = ( oldSize - sizeof( RouteInitialPayloadFinder_T ) ) / sizeof( RouteAddr_T );
	*newSize = ( MaxRouteFinderPacketSize > recCount ? oldSize : sizeof( RouteInitialPayloadFinder_T ) );
	*newSize += sizeof( RouteAddr_T );
	nextPayload = malloc( *newSize );

	if( NULL == nextPayload )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	newAddress = ( RouteAddr_T * )( nextPayload + 1 );

	if( MaxRouteFinderPacketSize > recCount ) {
		newAddress += recCount;
		memcpy( nextPayload, oldPayload, oldSize );
	} else
		nextPayload->MaxSize = MaxRouteFinderPacketSize;

	*newAddress = layer->LocalAddress;
	*newPayload = nextPayload;

	return FUNC_RESULT_SUCCESS;
}

int produceNextRouteFinder( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket,
							RouteAddr_T * nextHop, RouteStoredPacket_T * newPacket ) {
	if( NULL == layer || NULL == oldPacket || NULL == nextHop )
		return FUNC_RESULT_FAILED_ARGUMENT;

	CHECK_RESULT( midGenerate( &newPacket->InternalId, MoarLayer_Routing ) );

	newPacket->MessageId = oldPacket->MessageId;
	newPacket->Source = oldPacket->Source;
	newPacket->Destination = oldPacket->Destination;
	newPacket->XTL = oldPacket->XTL;

	newPacket->NextProcessing = 0;
	newPacket->PackType = RoutePackType_Finder;
	newPacket->State = StoredPackState_InProcessing;
	newPacket->TrysLeft = DEFAULT_ROUTE_TRYS;
	newPacket->NextHop = *nextHop;

	// todo push finderack back to origin if new chain started

	CHECK_RESULT( getNextFinderPayload( layer, oldPacket->Payload, oldPacket->PayloadSize, &( newPacket->Payload ), &( newPacket->PayloadSize ) ) );

	return FUNC_RESULT_SUCCESS;
}

