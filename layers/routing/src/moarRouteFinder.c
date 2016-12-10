//
// Created by spiralis on 05.11.16.
//

#include <moarRouteFinder.h>



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

    while( !hashIteratorIsLast( &iterator ) ) {
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
int getNextRouteFinderPayload(RoutingLayer_T *layer, uint8_t *prevPayload, PayloadSize_T prevPayloadSize,
                                  RouteAddr_T *nextHop, void **nextPacketPayload) {
    if( NULL == layer || NULL == prevPayload || 0 == prevPayloadSize || NULL == nextHop || NULL ==nextPacketPayload)
        return FUNC_RESULT_FAILED_ARGUMENT;

    uint8_t * nextPayload =  malloc(prevPayloadSize + sizeof(RouteAddr_T));
    if (NULL == nextPayload) return FUNC_RESULT_FAILED_MEM_ALLOCATION;

    memcpy(nextPayload, prevPayload, prevPayloadSize);
    *(RouteAddr_T*)(nextPayload + prevPayloadSize) = *nextHop;
    *nextPacketPayload =  nextPayload;
    return FUNC_RESULT_SUCCESS;
}


int produceNextRouteFinder(RoutingLayer_T *layer, RouteStoredPacket_T *prevPacket,
                           RouteAddr_T *nextHop, RouteStoredPacket_T *new_packet) {

    if( NULL == layer || NULL == prevPacket || NULL == nextHop)
        return FUNC_RESULT_FAILED_ARGUMENT;

    midGenerate(&new_packet->InternalId, MoarLayer_Routing);
    new_packet->MessageId = prevPacket->MessageId;

    new_packet->Source = layer->LocalAddress;
    new_packet->Destination = prevPacket->Destination;
    new_packet->NextProcessing = 0;
    new_packet->PackType = RoutePackType_Finder;
    new_packet->State = StoredPackState_InProcessing;
    new_packet->TrysLeft = DEFAULT_ROUTE_TRYS;
    new_packet->NextHop = *nextHop;
    new_packet->XTL = prevPacket->XTL;

    int recCount = (prevPacket->PayloadSize - sizeof(RouteInitialPayloadFinder_T)) / sizeof(RouteAddr_T); // todo get records count from prev packet as it may vary from packet to packet in different strategies
    if (MaxRouteFinderPacketSize <= recCount)
    {
// todo push finderack back to origin
        new_packet->PayloadSize = sizeof(RouteInitialPayloadFinder_T);
        new_packet->Payload = malloc(new_packet->PayloadSize);
        if (NULL == new_packet->Payload) return FUNC_RESULT_FAILED_MEM_ALLOCATION;
        RouteInitialPayloadFinder_T* payload = (RouteInitialPayloadFinder_T*)new_packet->Payload;
        payload->MaxSize = MaxRouteFinderPacketSize;
    }
    else {
        if (FUNC_RESULT_FAILED_MEM_ALLOCATION ==
            getNextRouteFinderPayload(layer, prevPacket->Payload, prevPacket->PayloadSize,
                                      nextHop, &new_packet->Payload))
            return FUNC_RESULT_FAILED_MEM_ALLOCATION;

        new_packet->PayloadSize = prevPacket->PayloadSize + sizeof(RouteAddr_T);
    }

    return FUNC_RESULT_SUCCESS;
}

