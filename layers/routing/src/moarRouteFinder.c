//
// Created by spiralis on 05.11.16.
//

#include <moarRouteFinder.h>
#include <moarRoutingStoredPacket.h>


int produceRouteFinder(RoutingLayer_T *layer, RouteAddr_T destination){
    if (NULL == layer)
        return FUNC_RESULT_FAILED_ARGUMENT;

    RouteStoredPacket_T packet = {0};

    midGenerate(&packet.InternalId, MoarLayer_Routing);
    rmidGenerate(&packet.MessageId);

    packet.Source = layer->LocalAddress;
    packet.Destination = destination;
    packet.NextProcessing = 0;
    packet.PackType = RoutePackType_Ack;
    packet.State = StoredPackState_InProcessing;
    packet.TrysLeft = DEFAULT_ROUTE_TRYS;
    RouteAddr_T nodes_list[1];
    nodes_list[0] = layer->LocalAddress;

    packet.PayloadSize = sizeof(RouteAddr_T) + sizeof(int);
    packet.Payload = malloc(packet.PayloadSize);
    if (NULL == packet.Payload) return FUNC_RESULT_FAILED_MEM_ALLOCATION;

    RoutePayloadFinder_T* payload = (RoutePayloadFinder_T*)packet.Payload;
    memcpy(payload->NodeList, nodes_list, sizeof(nodes_list));
    payload->Size = 1;

    return psAdd(&layer->PacketStorage, &packet);
}
