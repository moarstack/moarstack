//
// Created by svalov on 10/5/16.
//

#include <moarRoutingTablesHelper.h>
#include <funcResults.h>
#include <moarRouteTable.h>
#include <moarRoutingPrivate.h>

int resetTimeForDest(RoutingLayer_T* layer, ChannelAddr_T* dest){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == dest)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//foreach packet with same dest
	hashIterator_T iterator = {0};
	psGetDestFirst(&(layer->PacketStorage),dest, &iterator);
	while(!hashIteratorIsLast(&iterator)){
		RouteStoredPacket_T* pack = psIteratorData(&iterator);
		pack->NextProcessing = timeGetCurrent();
		psUpdateTime(&(layer->PacketStorage), pack);
		hashIteratorNext(&iterator);
	}
	hashIteratorFree(&iterator);
	return FUNC_RESULT_SUCCESS;
}
int helperAddRoute(RoutingLayer_T* layer, RouteAddr_T* dest, RouteAddr_T* relay){
	int addRes = RouteTableAdd(&layer->RouteTable, *relay, *dest);
	int resetRes = resetTimeForDest(layer, dest);
	return addRes;
}
int helperAddNeighbor(RoutingLayer_T* layer, ChannelAddr_T* address){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RoutingNeighborInfo_T neighborInfo = {0};
	neighborInfo.Address = *address;
	int res = storageAdd(&(layer->NeighborsStorage), &neighborInfo);
	if(FUNC_RESULT_SUCCESS!=res)
		return res;

	int resetRes = resetTimeForDest(layer, address);
	return res;
}
int helperRemoveRoute(RoutingLayer_T* layer, RouteAddr_T* dest, RouteAddr_T* relay){
	int delRes = RouteTableDelAll(&layer->RouteTable, *relay, *dest);
	return delRes;
}
int helperRemoveNeighbor(RoutingLayer_T* layer, ChannelAddr_T* address){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = storageRemove(&(layer->NeighborsStorage), address);
	return res;
}

int helperUpdateRoute( RoutingLayer_T * layer, RouteAddr_T * dest, RouteAddr_T * relay ) {
	RouteDataRecord_T	* row;
	int					result;

	if( NULL == layer || NULL == dest || NULL == relay )
		return FUNC_RESULT_FAILED_ARGUMENT;

	row = RouteTableGetRecord( &( layer->RouteTable ), *relay, *dest );

	if( NULL == row ) {
		RouteAddr_T	nullAddr = { 0 };

		RouteTableDelAll( &( layer->RouteTable ), nullAddr, *dest ); // remove all finder markers to that dest
		result = helperAddRoute( layer, dest, relay );
	} else {
		RouteTableUpdate( row );
		result = FUNC_RESULT_SUCCESS;
	}

	return result;
}

int helperUpdateNeighbor( RoutingLayer_T * layer ) {
	return FUNC_RESULT_FAILED;
}

int helperChannel2Route( ChannelAddr_T * channelAddr, RouteAddr_T * routeAddr ) {
	if( NULL == channelAddr || NULL == routeAddr )
		return FUNC_RESULT_FAILED_ARGUMENT;

	*routeAddr = *( RouteAddr_T * )channelAddr; // edit when types become different
	return FUNC_RESULT_SUCCESS;
}

int helperRoute2Channel( RouteAddr_T * routeAddr, ChannelAddr_T * channelAddr ) {
	if( NULL == channelAddr || NULL == routeAddr )
		return FUNC_RESULT_FAILED_ARGUMENT;

	*channelAddr = *( ChannelAddr_T * )routeAddr; // edit when types become different
	return FUNC_RESULT_SUCCESS;

}