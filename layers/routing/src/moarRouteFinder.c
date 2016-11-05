//
// Created by spiralis on 05.11.16.
//

#include <moarRoutingPrivate.h>
#include <moarRouteFinder.h>


int produceRouteFinder(RoutingLayer_T *layer){
    if (NULL == layer)
        return FUNC_RESULT_FAILED_ARGUMENT;

    RouteStoredPacket_T packet = {0};

    midGenerate(&packet.InternalId, MoarLayer_Routing);
    rmidGenerate(&packet.MessageId);

    packet.Source = layer->LocalAddress;
    packet.NextProcessing = 0;
    packet.PackType = RoutePackType_Ack;
    packet.State = StoredPackState_InProcessing;
    packet.TrysLeft = DEFAULT_ROUTE_TRYS;
    packet.PayloadSize = sizeof(RoutePayloadFinder_T);
    packet.Payload = malloc(packet.PayloadSize);
    if (NULL == packet.Payload) return FUNC_RESULT_FAILED_MEM_ALLOCATION;

    // todo fill payload




    return psAdd(&layer->PacketStorage, &packet);
}
