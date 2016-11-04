//
// Created by svalov on 10/6/16.
//

#include <funcResults.h>
#include <moarRoutingStoredPacket.h>
#include <moarRoutingPrivate.h>
#include <memory.h>
#include <moarRoutingStoredPacketFunc.h>
#include <moarCommons.h>
#include <moarRouteAck.h>
#include <moarRouteFinder.h>
#include <moarRoutingTablesHelper.h>
#include "moarRoutingPacketProcessing.h"

int notifyPresentation(RoutingLayer_T* layer, MessageId_T* id, PackStateRoute_T state){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == id)
		return FUNC_RESULT_FAILED_ARGUMENT;

	RouteMessageStateMetadata_T metadata = {0};
	metadata.Id = *id;
	metadata.State = state;
	LayerCommandStruct_T command = {0};
	command.Command = LayerCommandType_MessageState;
	command.MetaData = &metadata;
	command.MetaSize = sizeof(metadata);
	return WriteCommand(layer->PresentationSocket, &command);
}

int processReceivedDataPacket( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	RouteAddr_T relayAddr;
	int res;

	if( NULL == layer || NULL == packet || RoutePackType_Data != packet->PackType )
		return FUNC_RESULT_FAILED_ARGUMENT;

	res = helperChannel2Route( &( packet->LastHop ), &relayAddr );

	if( FUNC_RESULT_SUCCESS != res )
		return res;

	res = helperUpdateRoute( layer, &( packet->Source ), &relayAddr );

	if( FUNC_RESULT_SUCCESS != res )
		return res;

	// if destination
	if( routeAddrEqualPtr( &layer->LocalAddress, &packet->Destination ) ) {
		//// forward up
		res = sendPacketToPresentation( layer, packet );
		produceAck(layer,packet); // todo check function result
		//// dispose packet
		psRemove( &layer->PacketStorage, packet );
		res = FUNC_RESULT_SUCCESS;
	} else if( 0 < packet->XTL ) {// else if will be sent according to XTL
		//// change state to processing
		packet->State = StoredPackState_InProcessing;
		res = FUNC_RESULT_SUCCESS;
	}
	return res;
}

int processReceivedAckPacket( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	RouteAddr_T relayAddr;
	int res;

	if( NULL == layer || NULL == packet || RoutePackType_Ack != packet->PackType )
		return FUNC_RESULT_FAILED_ARGUMENT;

	res = helperChannel2Route( &( packet->LastHop ), &relayAddr );

	if( FUNC_RESULT_SUCCESS != res )
		return res;

	res = helperUpdateRoute( layer, &( packet->Source ), &relayAddr );

	if( FUNC_RESULT_SUCCESS != res )
		return res;

	// if destination
	if( routeAddrEqualPtr( &layer->LocalAddress, &packet->Destination ) ) {
		//// forward up event
		res = notifyPresentation( layer, &packet->InternalId,
								  PackStateRoute_Sent ); // todo review this logic !!! YES! Not by MID of ACK packet, but by source packet MID, which should be found by its RMID, retrieved from ACK packet payload
		//// dispose packet
		psRemove( &layer->PacketStorage, packet );
		//res = FUNC_RESULT_SUCCESS;
	} else if( 0 < packet->XTL ) {// else if will be sent according to XTL
		//// change state to processing
		packet->State = StoredPackState_InProcessing;
		res = FUNC_RESULT_SUCCESS;
	}
	return res;
}

int processReceivedFinderAckPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (RoutePackType_FinderAck != packet->PackType)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	// todo process content
	// if destination
	if(routeAddrEqualPtr(&layer->LocalAddress, &packet->Destination)) {
		res = FUNC_RESULT_SUCCESS;
	}else if( 0 < packet->XTL ) {// else if will be sent according to XTL
		//// todo create new packet
		//// todo add with processing state
	}
	//// dispose packet
	res = psRemove(&layer->PacketStorage, packet);
	return res;
}
int processReceivedFinderPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (RoutePackType_Finder != packet->PackType)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	// todo process content
	// if destination
	if(routeAddrEqualPtr(&layer->LocalAddress, &packet->Destination)) {
		//// todo create finder ack
		//// todo add finder ack with procesing state
	}else if( 0 < packet->XTL ) {// else if will be sent according to XTL
		//// todo create new packet
		//// todo try to send
		// also multiple stage finders process here
	}
	// dispose packet
	res = psRemove(&layer->PacketStorage, packet);
	return res;
}
int processReceivedProbePacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == packet)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (RoutePackType_Probe != packet->PackType)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	// todo update tables
	if( 0 < packet->XTL ) { // if will be sent according to XTL
		// todo create new probe
		// todo send probe
	}
	// dispose packet
	res = psRemove(&layer->PacketStorage, packet);
	return res;
}

int processReceivedPacket( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	if( NULL == layer || NULL == packet || StoredPackState_Received != packet->State )
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = FUNC_RESULT_FAILED;

	switch( packet->PackType ) {
		case RoutePackType_Data:
			res = processReceivedDataPacket( layer, packet );
			break;
		case RoutePackType_Ack:
			res = processReceivedAckPacket( layer, packet );
			break;
		case RoutePackType_Finder:
			res = processReceivedFinderPacket( layer, packet );
			break;
		case RoutePackType_FinderAck:
			res = processReceivedFinderAckPacket( layer, packet );
			break;
		case RoutePackType_Probe:
			res = processReceivedProbePacket( layer, packet );
			break;
		default:
			res = FUNC_RESULT_FAILED_ARGUMENT;
	}

	return res;
}

int processInProcessingPacket( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	if( NULL == layer || NULL == packet || StoredPackState_InProcessing != packet->State )
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = FUNC_RESULT_FAILED;

	if( DEC_XTL_ON_TRYS && DEFAULT_ROUTE_TRYS > packet->TrysLeft )
		packet->XTL -= DEFAULT_XTL_STEP;    // will decrease XTL on every attempt except of very first, if setting DEC_XTL_ON_TRYS is chosen TRUE

	packet->TrysLeft--;

	// if no trys left
	if( packet->TrysLeft <= 0 ) {
		if( RoutePackType_Data == packet->PackType && routeAddrEqualPtr( &layer->LocalAddress, &packet->Source ) ) {
			//// notify
			notifyPresentation( layer, &packet->InternalId, PackStateRoute_NotSent );
		} //else	
		// todo possible nack sending here
		//// dispose packet
		res = psRemove( &layer->PacketStorage, packet );
		return res;
	}
	// try to find route || neighbor
	// if found
	//// send to relay
	//// change state to wait sent
	//// update timeout
	// else
	//// update timeout
	//// send finder
	// todo make it started by if-else

	//todo fill stub local adresses, replace with actual addrs.
	produceRouteFinder(layer,  layer->LocalAddress, layer->LocalAddress);

	return res;
}

int processWaitSentPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	if(StoredPackState_WaitSent != packet->State)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// timeout while send
	// change packet state to processing
	packet->State = StoredPackState_InProcessing;
	return FUNC_RESULT_SUCCESS;
}
int processWaitAckPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	if(StoredPackState_WaitAck != packet->State)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// timeout while waiting ack
	// todo decrease route metric
	// set to processing
	packet->State = StoredPackState_InProcessing;
	return FUNC_RESULT_SUCCESS;
}
int processDisposedPacket(RoutingLayer_T* layer, RouteStoredPacket_T* packet){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == packet)
		return  FUNC_RESULT_FAILED_ARGUMENT;
	if(StoredPackState_Disposed != packet->State)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = psRemove(&layer->PacketStorage,packet);
	return res;
}

int processPacketStorage(RoutingLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RouteStoredPacket_T* packet = psGetTop(&layer->PacketStorage);
	while(NULL != packet && timeCompare(packet->NextProcessing, timeGetCurrent()) <= 0){
		// logic
		int res = FUNC_RESULT_FAILED;
		switch(packet->State) {
			//if new packet
			case StoredPackState_Received:
				res = processReceivedPacket(layer, packet);
				break;
			// if data already processed
			case StoredPackState_WaitSent:
				res = processWaitSentPacket(layer, packet);
				break;
			case StoredPackState_WaitAck:
				res = processWaitAckPacket(layer, packet);
				break;
			case StoredPackState_InProcessing:
				res = processInProcessingPacket(layer, packet);
				break;
			case StoredPackState_Disposed:
				res = processDisposedPacket(layer, packet);
				break;
			default:
				res = FUNC_RESULT_FAILED;
		}
		if(FUNC_RESULT_SUCCESS != res)
			return res;
		packet = psGetTop(&layer->PacketStorage);
	}
	return FUNC_RESULT_SUCCESS;
}