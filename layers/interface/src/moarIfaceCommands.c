//
// Created by kryvashek on 13.08.16.
//

#include <moarIfaceCommands.h>
#include <moarInterfacePrivate.h>

int clearCommand( IfaceState_T * layer ) {
	return FreeCommand( &( layer->Memory.Command ) );
}

int processCommandIface( IfaceState_T * layer, LayerCommandType_T commandType, void * metaData, void * data, size_t dataSize ) {
	static size_t	sizes[ LayerCommandType_TypesCount ] = { 0,	// LayerCommandType_None
															  0,	// LayerCommandType_Send
															  IFACE_RECEIVE_METADATA_SIZE,	// LayerCommandType_Receive
															  IFACE_NEIGHBOR_METADATA_SIZE,	// LayerCommandType_NewNeighbor
															  IFACE_NEIGHBOR_METADATA_SIZE,	// LayerCommandType_LostNeighbor
															  IFACE_NEIGHBOR_METADATA_SIZE,	// LayerCommandType_UpdateNeighbor
															  IFACE_PACK_STATE_METADATA_SIZE,	// LayerCommandType_MessageState
															  IFACE_REGISTER_METADATA_SIZE,	// LayerCommandType_RegisterInterface
															  0,	// LayerCommandType_RegisterInterfaceResult
															  IFACE_UNREGISTER_METADATA_SIZE,	// LayerCommandType_UnregisterInterface
															  0,	// LayerCommandType_ConnectApplication
															  0,	// LayerCommandType_ConnectApplicationResult
															  0,	// LayerCommandType_DisconnectApplication
															  IFACE_MODE_STATE_METADATA_SIZE,	// LayerCommandType_InterfaceState
															  0	// LayerCommandType_UpdateBeaconPayload
	};

	clearCommand( layer );
	layer->Memory.Command.Command = commandType;
	layer->Memory.Command.MetaSize = sizes[ commandType ];
	layer->Memory.Command.MetaData = metaData;
	layer->Memory.Command.DataSize = dataSize;
	layer->Memory.Command.Data = data;

	return writeUp( layer );
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
	layer->Config.IsWaitingForResponse = false;

	if( IFACE_BEACON_INTERVAL > IFACE_RESPONSE_WAIT_INTERVAL )
		layer->Config.BeaconIntervalCurrent = IFACE_BEACON_INTERVAL - IFACE_RESPONSE_WAIT_INTERVAL;

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
	result = processCommandIface( layer, LayerCommandType_NewNeighbor, &metadata, NULL, 0 );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceNeighborNew()" );

	return result;
}

int processCommandIfaceNeighborUpdate( IfaceState_T * layer, IfaceAddr_T * address ) {
	int						result;
	IfaceNeighborMetadata_T	metadata;

	metadata.Neighbor = *address;
	result = processCommandIface( layer, LayerCommandType_UpdateNeighbor, &metadata, NULL, 0 );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceNeighborUpdate()" );

	return result;
}

int processCommandIfaceReceived( IfaceState_T * layer ) {
	int						result;
	IfaceReceiveMetadata_T	metadata;

	result = midGenerate( &( metadata.Id ), MoarLayer_Interface );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	metadata.From = layer->Memory.BufferHeader.From;
	result = processCommandIface( layer, LayerCommandType_Receive, &metadata, layer->Memory.Buffer, layer->Memory.BufferHeader.Size );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandIfaceReceived()" );

	return result;
}

int processCommandChannelRegisterResult( IfaceState_T * layer ) {
	int result;

	layer->Config.IsConnectedToChannel = ( ( ChannelRegisterResultMetadata_T * ) layer->Memory.Command.MetaData )->Registred;
	clearCommand( layer );

	if( layer->Config.IsConnectedToChannel ) {
		LogWrite( layer->Config.LogHandle, LogLevel_Information, "interface registered in channel layer" );
		result = FUNC_RESULT_SUCCESS;
	} else
		result = processCommandIfaceRegister( layer );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandChannelRegisterResult()" );

	return result;
}

int processCommandChannelSend( IfaceState_T * layer ) {
	IfaceNeighbor_T			* neighbor;
	ChannelSendMetadata_T	* metadata;
	int						result;

	metadata = ( ChannelSendMetadata_T * ) layer->Memory.Command.MetaData;
	neighbor = neighborFind( layer, &( metadata->To ) );
	layer->Memory.ProcessingMessageId = metadata->Id;

	if( NULL == neighbor )
		result = processCommandIfaceUnknownDest( layer );
	else {
		result = transmitMessage( layer, neighbor, metadata->NeedResponse );

		if( FUNC_RESULT_SUCCESS == result ) {
			if( metadata->NeedResponse ) {
				layer->Config.BeaconIntervalCurrent = IFACE_RESPONSE_WAIT_INTERVAL;
				layer->Config.IsWaitingForResponse = true;
			} else
				result = processCommandIfaceMessageSent( layer );
		}
	}

	clearCommand( layer );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandChannelSend()" );

	return result;
}

int processCommandChannelUpdateBeacon( IfaceState_T * layer ) {
	int result = FUNC_RESULT_SUCCESS;

	if( ( NULL == layer->Memory.Command.Data && 0 < layer->Memory.Command.DataSize ) ||
		( NULL != layer->Memory.Command.Data && 0 == layer->Memory.Command.DataSize ) ||
		IFACE_MAX_PAYLOAD_BEACON_SIZE < layer->Memory.Command.DataSize )
		result = FUNC_RESULT_FAILED_ARGUMENT;

	if( FUNC_RESULT_SUCCESS == result ) {
		memcpy( layer->Memory.BeaconPayload, layer->Memory.Command.Data, layer->Memory.Command.DataSize );
		layer->Config.BeaconPayloadSize = layer->Memory.Command.DataSize;
	} else
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "processCommandChannelUpdateBeacon()" );

	clearCommand( layer );
	return result;
}

int processCommandChannel( IfaceState_T * layer ) {
	int	result;

	result = readUp( layer );

	if( FUNC_RESULT_SUCCESS == result ) {
		if( layer->Config.IsConnectedToChannel )
			switch( layer->Memory.Command.Command ) {
				case LayerCommandType_Send :
					result = processCommandChannelSend( layer );
					break;

				case LayerCommandType_UpdateBeaconPayload :
					result = processCommandChannelUpdateBeacon( layer );
					break;

				default :
					result = FUNC_RESULT_FAILED_ARGUMENT;
					LogWrite( layer->Config.LogHandle, LogLevel_Warning, "interface got unknown command %d from channel", layer->Memory.Command.Command );
			}
		else if( LayerCommandType_RegisterInterfaceResult == layer->Memory.Command.Command )
			result = processCommandChannelRegisterResult( layer );
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