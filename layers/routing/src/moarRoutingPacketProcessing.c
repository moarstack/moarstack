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
#include <moarRouteFinderAck.h>
#include <moarRoutingTablesHelper.h>
#include "moarRoutingPacketProcessing.h"
#include <moarRouteProbe.h>
#include <moarRouteFinderAck.h>
#include <moarRoutingNeighborsStorage.h>
#include <hashTable.h>

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
	int 		result;

	if( NULL == layer || NULL == packet || RoutePackType_Data != packet->PackType )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( !routeAddrEqualPtr( &layer->LocalAddress, &packet->Source ) ) {
		result = helperChannel2Route( &( packet->LastHop ), &relayAddr );
		CHECK_RESULT( result );

		result = helperUpdateRoute( layer, &( packet->Source ), &relayAddr );
		CHECK_RESULT( result );
	}

	// if destination
	if( routeAddrEqualPtr( &layer->LocalAddress, &packet->Destination ) ) {
		//// forward up
		result = sendPacketToPresentation( layer, packet );
		CHECK_RESULT( result );

		if( routeAddrEqualPtr( &layer->LocalAddress, &packet->Source ) )
			result = notifyPresentation( layer, &( packet->InternalId ), PackStateRoute_Received );
		else
			result = produceAck( layer, packet );

		CHECK_RESULT( result );

		//// dispose packet
		result = psRemove( &layer->PacketStorage, packet );
	} else if( 0 < packet->XTL ) {// else if will be sent according to XTL
		//// change state to processing
		packet->State = StoredPackState_InProcessing;
		result = FUNC_RESULT_SUCCESS;
	} else {
		packet->State = StoredPackState_Disposed;
		result = FUNC_RESULT_SUCCESS;
	}

	return result;
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
		RoutingMessageId_T	* sourceDataRmid = &( ( RoutePayloadAck_T * )packet->Payload )->messageId;
		RouteStoredPacket_T	* sourcePacket;

		sourcePacket = psGetRmidPtr( &( layer->PacketStorage ), sourceDataRmid );
		res = FUNC_RESULT_SUCCESS;

		if( NULL != sourcePacket ) {
			res = notifyPresentation( layer, &sourcePacket->InternalId, PackStateRoute_Sent ); // forward up event  // TODO check result
			psRemove( &layer->PacketStorage, sourcePacket ); // dispose source packet
		}

		psRemove( &layer->PacketStorage, packet ); // dispose packet
	} else if( 0 < packet->XTL ) {// else if will be sent according to XTL
		//// change state to processing
		packet->State = StoredPackState_InProcessing;
		res = FUNC_RESULT_SUCCESS;
	} else {
		packet->State = StoredPackState_Disposed;
		res = FUNC_RESULT_SUCCESS;
	}
	
	return res;
}

int processReceivedFinderAckPacket( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	int result;

	if( NULL == layer || NULL == packet || RoutePackType_FinderAck != packet->PackType )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = processPayloadFack( layer, packet );

	if( FUNC_RESULT_SUCCESS != result ) {
		result = psRemove( &layer->PacketStorage, packet ); // dispose packet
		return result;
	}

	if( routeAddrEqualPtr( &layer->LocalAddress, &packet->Destination ) ) // if destination
		result = psRemove( &layer->PacketStorage, packet ); // dispose packet
	else if( 0 < packet->XTL ) { // else if will be sent according to XTL
		packet->State = StoredPackState_InProcessing;
		result = FUNC_RESULT_SUCCESS;
	} else {
		packet->State = StoredPackState_Disposed;
		result = FUNC_RESULT_SUCCESS;
	}

	return result;
}

int sendFinderFurther( RoutingLayer_T * layer, RouteStoredPacket_T * oldPacket ) {
	RouteStoredPacket_T	newPacket;
	int					result;

	CHECK_RESULT( produceNextRouteFinder( layer, oldPacket, &newPacket ) );
	result = helperFindRelay( layer, &( newPacket.Destination ), &( newPacket.NextHop ) );

	if( FUNC_RESULT_SUCCESS == result )
		result = sendPacketToChannel( layer, &newPacket );
	else if( FUNC_RESULT_FAILED_NEIGHBORS == result ) {
		RoutingNeighborInfo_T	* neInfo;
		hashIterator_T			iterator;
		int						count = 0;

		result = hashIterator( &( layer->NeighborsStorage.Storage ), &iterator );

		if( FUNC_RESULT_SUCCESS == result ) {
			while( !hashIteratorEnded( &iterator ) ) {
				neInfo = hashIteratorData( &iterator );
				result = helperRoute2Channel( &( neInfo->Address ), &( newPacket.NextHop ) );

				if( FUNC_RESULT_SUCCESS == result )
					result = sendPacketToChannel( layer, &newPacket );

				if( FUNC_RESULT_SUCCESS == result )
					count++;

				result = hashIteratorNext( &iterator );

				if( FUNC_RESULT_SUCCESS != result )
					break;
			}

			result = ( 0 < count ? FUNC_RESULT_SUCCESS : FUNC_RESULT_FAILED );
		}
	}

	CHECK_RESULT( clearStoredPacket( &newPacket ) );
	return result;
}

int processReceivedFinderPacket( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	int result;

	if( NULL == layer || NULL == packet || RoutePackType_Finder != packet->PackType || 0 == packet->PayloadSize ||
		NULL == packet->Payload )
		return FUNC_RESULT_FAILED_ARGUMENT;

	// todo process content

	if( routeAddrEqualPtr( &layer->LocalAddress, &packet->Destination ) ) // if destination
		result = sendFack( layer, packet );
	else if( 0 < packet->XTL ) // else if will be sent according to XTL
		result = sendFinderFurther( layer, packet );
	else
		result = FUNC_RESULT_SUCCESS; // packet disposing due to XTL, its normal

	CHECK_RESULT( psRemove( &( layer->PacketStorage ), packet ) ); // dispose packet
	return result;
	}

int processReceivedProbePacket( RoutingLayer_T * layer, RouteStoredPacket_T * packet ) {
	int result;

	if( NULL == layer || NULL == packet || RoutePackType_Probe != packet->PackType )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = processProbePacket(layer, packet);
//	CHECK_RESULT( result );

	if( 0 < packet->XTL ) // if will be sent according to XTL
		result = sendProbeNext( layer, packet );

	if( FUNC_RESULT_SUCCESS == result )
		result = psRemove( &layer->PacketStorage, packet ); // dispose packet
	else {
		packet->NextProcessing = timeAddInterval( timeGetCurrent(), DEFAULT_PROBE_SEND_PERIOD );
		psUpdateTime( &( layer->PacketStorage ), packet );
	}
	return result;
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
	ChannelAddr_T 		relayAddr;
	moarTimeInterval_T	interval;
	int					result;

	if( NULL == layer || NULL == packet || StoredPackState_InProcessing != packet->State )
		return FUNC_RESULT_FAILED_ARGUMENT;

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
		result = psRemove( &layer->PacketStorage, packet );
		return result;
	}

	result = helperFindRelay( layer, &( packet->Destination ), &relayAddr ); // try to find route || neighbor

	if( FUNC_RESULT_SUCCESS == result ) {// if found
		packet->NextHop = relayAddr;
		result = sendPacketToChannel( layer, packet );	// send to relay
		interval = ( FUNC_RESULT_SUCCESS == result ? SENT_WAITING_TIMEOUT : UNSENT_WAITING_TIMEOUT );

		if( FUNC_RESULT_SUCCESS == result )
			packet->State = StoredPackState_WaitSent;	// change state to wait sent
	} else {// else
		result = sendFindersFirst( layer, &( packet->Destination ) ); // send finders
		interval = ( FUNC_RESULT_SUCCESS == result ? FACK_WAITING_TIMEOUT : UNSENT_WAITING_TIMEOUT );
	}

	packet->NextProcessing = timeAddInterval( timeGetCurrent(), interval );
	result = psUpdateTime( &( layer->PacketStorage ), packet );	// update timeout

	return result;
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