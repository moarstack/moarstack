//
// Created by kryvashek on 13.08.16.
//

#include <moarIfaceCommands.h>

size_t commandMetaSize( LayerCommandType_T type ) {
	size_t	size;

	switch( type ) {
		case LayerCommandType_Receive : size = IFACE_RECEIVE_METADATA_SIZE; break;
		case LayerCommandType_NewNeighbor :
		case LayerCommandType_LostNeighbor :
		case LayerCommandType_UpdateNeighbor : size = IFACE_NEIGHBOR_METADATA_SIZE; break;
		case LayerCommandType_MessageState : size = IFACE_PACK_STATE_METADATA_SIZE; break;
		case LayerCommandType_RegisterInterface : size = IFACE_REGISTER_METADATA_SIZE; break;
		case LayerCommandType_UnregisterInterface : size = IFACE_UNREGISTER_METADATA_SIZE; break;
		case LayerCommandType_InterfaceState : size = IFACE_MODE_STATE_METADATA_SIZE; break;
		default : size = 0;
	}

	return size;
}

int processCommandIface( IfaceState_T * layer, LayerCommandType_T commandType, void * metaData, void * data, size_t dataSize ) {
	LayerCommandStruct_T	command = { 0 };
	
	if( NULL == layer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	command.Command = commandType;
	command.MetaSize = commandMetaSize( commandType );
	command.MetaData = metaData;
	command.DataSize = dataSize;
	command.Data = data;

	return writeUp( layer, &command );
}

int processCommandIfaceRegister( IfaceState_T * layer ) {
	IfaceRegisterMetadata_T	metadata;
	int						result;

	metadata.Length = IFACE_ADDR_SIZE;
	metadata.Value = layer->Config.Address;
	result = processCommandIface( layer, LayerCommandType_RegisterInterface, &metadata, NULL, 0 );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceRegister()" );

	return result;
}

int processCommandIfaceUnknownDest( IfaceState_T * layer ) {
	int							result;
	IfacePackStateMetadata_T	metadata;

	metadata.Id = layer->Memory.ProcessingMessageId;
	metadata.State = IfacePackState_UnknownDest;
	result = processCommandIface( layer, LayerCommandType_MessageState, &metadata, NULL, 0 );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceUnknownDest()" );

	return result;
}

int processCommandIfaceTimeoutFinished( IfaceState_T * layer, bool gotResponse ) {
	IfacePackStateMetadata_T	metadata;
	int 						result;

	metadata.Id = layer->Memory.ProcessingMessageId;
	metadata.State = ( gotResponse ? IfacePackState_Responsed : IfacePackState_Timeouted );
	result = processCommandIface( layer, LayerCommandType_MessageState, &metadata, NULL, 0 );

	if( !gotResponse && NULL != layer->Memory.LastSent && 0 < layer->Memory.LastSent->AttemptsLeft )
		layer->Memory.LastSent->AttemptsLeft--;

	layer->Memory.LastSent = NULL;
	layer->Config.IsWaitingForResponse = false;

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceTimeoutFinished()" );

	return result;
}

int processCommandIfaceMessageSent( IfaceState_T * layer ) {
	int							result;
	IfacePackStateMetadata_T	metadata;

	metadata.Id = layer->Memory.ProcessingMessageId;
	metadata.State = IfacePackState_Sent;
	result = processCommandIface( layer, LayerCommandType_MessageState, &metadata, NULL, 0 );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceMessageSent()" );

	return result;
}

int processCommandIfaceNeighborNew( IfaceState_T * layer, IfaceAddr_T * address ) {
	int						result;
	IfaceNeighborMetadata_T	metadata;

	metadata.Neighbor = *address;
	result = processCommandIface( layer, LayerCommandType_NewNeighbor, &metadata, layer->Memory.Payload, layer->Memory.BufferHeader.Size );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceNeighborNew()" );

	return result;
}

int processCommandIfaceNeighborUpdate( IfaceState_T * layer, IfaceAddr_T * address ) {
	int						result;
	IfaceNeighborMetadata_T	metadata;

	metadata.Neighbor = *address;
	result = processCommandIface( layer, LayerCommandType_UpdateNeighbor, &metadata, layer->Memory.Payload, layer->Memory.BufferHeader.Size );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceNeighborUpdate()" );

	return result;
}

int processCommandIfaceNeighborLost( IfaceState_T * layer, IfaceAddr_T * address ) {
	int						result;
	IfaceNeighborMetadata_T	metadata;

	metadata.Neighbor = *address;
	result = processCommandIface( layer, LayerCommandType_LostNeighbor, &metadata, NULL, 0 );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceNeighborLost()" );

	return result;
}

int processCommandIfaceReceived( IfaceState_T * layer ) {
	int						result;
	IfaceReceiveMetadata_T	metadata;

	result = midGenerate( &( metadata.Id ), MoarLayer_Interface );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	metadata.From = layer->Memory.BufferHeader.From;
	result = processCommandIface( layer, LayerCommandType_Receive, &metadata, layer->Memory.Payload, layer->Memory.BufferHeader.Size );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceReceived()" );

	return result;
}

int processCommandChannelRegisterResult( IfaceState_T * layer, LayerCommandStruct_T * command ) {
	int result;

	if( NULL == layer || NULL == command )
		return FUNC_RESULT_FAILED_ARGUMENT;

	layer->Config.IsConnectedToChannel = ( ( ChannelRegisterResultMetadata_T * ) command->MetaData )->Registred;
	FreeCommand( command );

	if( layer->Config.IsConnectedToChannel ) {
		LogWrite( layer->Config.LogHandle, LogLevel_Information, "interface registered in channel layer" );
		result = FUNC_RESULT_SUCCESS;
	} else
		result = processCommandIfaceRegister( layer );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandChannelRegisterResult()" );

	return result;
}

int processCommandChannelSend( IfaceState_T * layer, LayerCommandStruct_T * command ) {
	IfaceNeighbor_T			* neighbor;
	ChannelSendMetadata_T	* metadata;
	int						result;

	metadata = ( ChannelSendMetadata_T * ) command->MetaData;
	neighbor = neighborFind( layer, &( metadata->To ) );
	layer->Memory.ProcessingMessageId = metadata->Id;

	if( NULL == neighbor )
		result = processCommandIfaceUnknownDest( layer );
	else {
		result = transmitMessage( layer, neighbor, metadata->NeedResponse, command );

		if( FUNC_RESULT_SUCCESS == result ) {
			if( metadata->NeedResponse ) {
				layer->Config.IsWaitingForResponse = true;
				layer->Memory.LastNeedResponse = timeGetCurrent();
				layer->Memory.LastSent = neighbor;
			} else
				result = processCommandIfaceMessageSent( layer );
		}
	}

	FreeCommand( command );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandChannelSend()" );

	return result;
}

int processCommandChannelUpdateBeacon( IfaceState_T * layer, LayerCommandStruct_T * command ) {
	int result = FUNC_RESULT_SUCCESS;

	if( NULL == layer || NULL == command ||
		( NULL == command->Data && 0 < command->DataSize ) ||
		( NULL != command->Data && 0 == command->DataSize ) ||
		IFACE_MAX_PAYLOAD_BEACON_SIZE < command->DataSize )
		result = FUNC_RESULT_FAILED_ARGUMENT;

	if( FUNC_RESULT_SUCCESS == result ) {
		memcpy( layer->Memory.BeaconPayload, command->Data, command->DataSize );
		layer->Config.BeaconPayloadSize = command->DataSize;
	} else
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandChannelUpdateBeacon()" );

	FreeCommand( command );
	return result;
}

int processCommandChannel( IfaceState_T * layer ) {
	LayerCommandStruct_T	command = { 0 };
	int						result;

	result = readUp( layer, &command );

	if( FUNC_RESULT_SUCCESS == result ) {
		if( layer->Config.IsConnectedToChannel )
			switch( command.Command ) {
				case LayerCommandType_Send :
					result = processCommandChannelSend( layer, &command );
					break;

				case LayerCommandType_UpdateBeaconPayload :
					result = processCommandChannelUpdateBeacon( layer, &command );
					break;

				default :
					result = FUNC_RESULT_FAILED_ARGUMENT;
					LogWrite( layer->Config.LogHandle, LogLevel_Warning, "interface got unknown command %d from channel", command.Command );
			}
		else if( LayerCommandType_RegisterInterfaceResult == command.Command )
			result = processCommandChannelRegisterResult( layer, &command );
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandChannel()" );

	return result;
}

int processChannelConnection( IfaceState_T * layer ) {
	int					result;
	struct epoll_event	* event;

	result = connectUp( layer );

	if( FUNC_RESULT_SUCCESS == result ) {
		event = layer->Memory.EpollEvents + IFACE_ARRAY_CHANNEL_POSITION;
		event->data.fd = layer->Config.ChannelSocket;
		event->events = EPOLLIN | EPOLLHUP | EPOLLERR;
		result = epoll_ctl( layer->Config.EpollHandler, EPOLL_CTL_ADD, layer->Config.ChannelSocket, event );
	}

	if( FUNC_RESULT_SUCCESS == result )
		result = processCommandIfaceRegister( layer );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Critical, result, "processChannelConnection()" );

	return result;
}

int processChannelReconnection( IfaceState_T * layer, bool forced ) {
	int					result;
	struct epoll_event	* event;

	event = layer->Memory.EpollEvents + IFACE_ARRAY_CHANNEL_POSITION;
	result = epoll_ctl( layer->Config.EpollHandler, EPOLL_CTL_DEL, layer->Config.ChannelSocket, event );

	if( FUNC_RESULT_SUCCESS == result ) {
		if( forced ) {
			shutdown( layer->Config.ChannelSocket, SHUT_RDWR );
			close( layer->Config.ChannelSocket );
		}

		layer->Config.IsConnectedToChannel = false;
		result = processChannelConnection( layer );
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Critical, result, "processChannelReconnection()" );

	return result;
}