//
// Created by svalov, kryvashek on 05.07.16.
//

#include <moarInterfacePrivate.h>

static IfaceState_T	state = { 0 };

int writeDown( void * buffer, size_t bytes ) {
	ssize_t result;

	if( ( NULL == buffer && 0 < bytes ) ||
		FUNC_RESULT_SUCCESS >= state.Config.MockitSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = write( state.Config.MockitSocket, buffer, bytes );

		if( result == bytes )
			return FUNC_RESULT_SUCCESS;
		else if( 1 < IFACE_SEND_ATTEMPTS_COUNT )
			sleep( IFACE_MOCKIT_WAIT_INTERVAL );
	}

	return FUNC_RESULT_FAILED_IO;
}

ssize_t readDown( void * buffer, size_t bytes ) {
	ssize_t	result;

	if( ( NULL == buffer && 0 < bytes ) ||
		FUNC_RESULT_SUCCESS >= state.Config.MockitSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = read( state.Config.MockitSocket, buffer, bytes );

		if( 0 < result )
			break;
		else if( 1 < IFACE_SEND_ATTEMPTS_COUNT )
			sleep( IFACE_MOCKIT_WAIT_INTERVAL );
	}

	return result;
}

int connectDown( void ) {
	int	result;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = SocketOpenFile( IFACE_MOCKIT_SOCKET_FILE, false, &( state.Config.MockitSocket ) );

		if( FUNC_RESULT_SUCCESS == result )
			break;
		else if( 1 < IFACE_SEND_ATTEMPTS_COUNT )
			sleep( IFACE_MOCKIT_WAIT_INTERVAL );
	}

	return result;
}

int preparePhysically( void ) {
	struct timeval	moment;
	int				result,
					length,
					address;

	result = connectDown();

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	gettimeofday( &moment, NULL );
	srand( ( unsigned int )( moment.tv_usec ) );

	do {
		address = 1 + rand() % IFACE_ADDRESS_LIMIT;
		snprintf( state.Memory.Buffer, IFACE_BUFFER_SIZE, "%d%n", address, &length );
		result = writeDown( state.Memory.Buffer, length );

		if( FUNC_RESULT_SUCCESS != result )
			return FUNC_RESULT_FAILED_IO;

		result = readDown( state.Memory.Buffer, IFACE_BUFFER_SIZE );

		if( FUNC_RESULT_SUCCESS >= result )
			return FUNC_RESULT_FAILED_IO;
	} while( 0 != strncmp( IFACE_REGISTRATION_OK, state.Memory.Buffer, strlen( IFACE_REGISTRATION_OK ) ) );

	memcpy( &( state.Config.Address ), &address, IFACE_ADDR_SIZE );
	printf( "Interface registered in MockIT with address %d\n", address );
	fflush( stdout );

	return FUNC_RESULT_SUCCESS;
}

int writeUp( void ) {
	int result;

	for( int attempt = 0; attempt < IFACE_PUSH_ATTEMPTS_COUNT; attempt++ ) {
		result = WriteCommand( state.Config.ChannelSocket, &( state.Memory.Command ) );

		if( FUNC_RESULT_SUCCESS == result )
			break;
		else if( 1 < IFACE_PUSH_ATTEMPTS_COUNT )
			sleep( IFACE_CHANNEL_WAIT_INTERVAL );
	}

	return result;
}

int readUp( void ) {
	int result;

	for( int attempt = 0; attempt < IFACE_PUSH_ATTEMPTS_COUNT; attempt++ ) {
		result = ReadCommand( state.Config.ChannelSocket, &( state.Memory.Command ) );

		if( FUNC_RESULT_SUCCESS == result )
			break;
		else if( 1 < IFACE_PUSH_ATTEMPTS_COUNT )
			sleep( IFACE_CHANNEL_WAIT_INTERVAL );
	}

	return result;
}

int connectUp( void ) {
	int	result = FUNC_RESULT_FAILED;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = SocketOpenFile( state.Config.ChannelSocketFilepath, false, &( state.Config.ChannelSocket ) );

		if( FUNC_RESULT_SUCCESS == result )
			break;
		else if( 1 < IFACE_PUSH_ATTEMPTS_COUNT )
			sleep( IFACE_CHANNEL_WAIT_INTERVAL );
	}

	return result;
}

IfaceNeighbor_T * neighborFind( IfaceAddr_T * address ) {
	for( int i = 0; i < state.Config.NeighborsCount; i++ )
		if( 0 == memcmp( address, &( state.Memory.Neighbors[ i ].Address ), IFACE_ADDR_SIZE ) )
			return state.Memory.Neighbors + i;

	return NULL;
}

int neighborAdd( IfaceAddr_T * address, PowerFloat_T minPower ) {
	if( NULL == address )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( IFACE_MAX_NEIGHBOR_COUNT == state.Config.NeighborsCount )
		return FUNC_RESULT_FAILED;

	state.Memory.Neighbors[ state.Config.NeighborsCount ].Address = *address;
	state.Memory.Neighbors[ state.Config.NeighborsCount ].MinPower = minPower;
	state.Memory.Neighbors[ state.Config.NeighborsCount ].AttemptsLeft = IFACE_SEND_ATTEMPTS_COUNT;
	state.Memory.Neighbors[ state.Config.NeighborsCount ].LinkQuality = IFACE_DEFAULT_LINK_QUALITY;

	return FUNC_RESULT_SUCCESS;
}

int neighborRemove( IfaceNeighbor_T * neighbor ) {
	ptrdiff_t	toPreserve;

	if( NULL == neighbor )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( 0 == state.Config.NeighborsCount )
		return FUNC_RESULT_FAILED;

	toPreserve = ( state.Memory.Neighbors + state.Config.NeighborsCount ) - ( neighbor + 1 );
	memmove( neighbor, neighbor + 1, toPreserve * IFACE_NEIGHBOR_SIZE );
	state.Config.NeighborsCount--;
	memset( state.Memory.Neighbors + state.Config.NeighborsCount, 0, IFACE_NEIGHBOR_SIZE );

	return FUNC_RESULT_SUCCESS;
}

int neighborUpdate( IfaceNeighbor_T * neighbor, PowerFloat_T newMinPower ) {
	if( newMinPower < neighbor->MinPower && neighbor->AttemptsLeft < IFACE_SEND_ATTEMPTS_COUNT )
		neighbor->AttemptsLeft++; // neighbor became closer

	neighbor->MinPower = newMinPower;
	return FUNC_RESULT_SUCCESS;
}


int getFloat( PowerFloat_T * value, char * buffer, ssize_t * bytesLeft ) {
	char	* end;

	if( NULL == buffer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( NULL == value )
		strtof( buffer, &end );
	else
		*value = strtof( buffer, &end );

	if( NULL != bytesLeft && 0 < *bytesLeft )
		*bytesLeft -= ( end - buffer );

	if( 0 == end - buffer )
		return FUNC_RESULT_FAILED;

	return FUNC_RESULT_SUCCESS;
}

int receiveDataPiece( void * destination, ssize_t expectedSize, char ** bufStart, ssize_t * bytesLeft ) {
	ssize_t	bytesDone = 0;

	if( NULL == bufStart || NULL == *bufStart || NULL == bytesLeft )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( 0 < *bytesLeft ) {
		bytesDone = ( *bytesLeft < expectedSize ? *bytesLeft : expectedSize );
		memcpy( destination, *bufStart, bytesDone );
		*bufStart += bytesDone;
		*bytesLeft -= bytesDone;
	}

	if( bytesDone < expectedSize )
		bytesDone += readDown( ( char * )destination + bytesDone, expectedSize - bytesDone );

	if( bytesDone < expectedSize )
		return FUNC_RESULT_FAILED_IO;

	return FUNC_RESULT_SUCCESS;
}

int receiveAnyData( PowerFloat_T * finishPower) {
	ssize_t	bytesLeft,
			result;
	char	* buffer = state.Memory.Buffer;

	if( NULL == finishPower )
		return FUNC_RESULT_FAILED_ARGUMENT;

	bytesLeft = readDown( buffer, IFACE_BUFFER_SIZE );

	if( -1 == bytesLeft )
		return FUNC_RESULT_FAILED_IO;

	result = getFloat( finishPower, buffer, &bytesLeft ); // we assume that buffer is big enough to contain float

	if( 0 == result )
		return FUNC_RESULT_FAILED_IO; // the strange format is detected if no float value precedes the data, so reading is impossible

	bytesLeft--; // extra byte is for space (aka delimiter) symbol
	buffer++;
	result = receiveDataPiece( &( state.Memory.BufferHeader ), IFACE_HEADER_SIZE, &buffer, &bytesLeft );

	if( FUNC_RESULT_SUCCESS != result && 0 < state.Memory.BufferHeader.Size )
		result = receiveDataPiece( state.Memory.Payload, state.Memory.BufferHeader.Size, &buffer, &bytesLeft );

	if( FUNC_RESULT_SUCCESS != result && IfacePackType_Beacon == state.Memory.BufferHeader.Type )
		result = receiveDataPiece( &( state.Memory.BufferFooter ), IFACE_FOOTER_SIZE, &buffer, &bytesLeft );

	if( FUNC_RESULT_SUCCESS != result )
		return FUNC_RESULT_FAILED_IO;

	return FUNC_RESULT_SUCCESS;
}

int transmitAnyData( PowerFloat_T power, void * data, size_t size ) {
	int		currentLength,
			result;

	if( ( NULL == data && 0 < size ) || ( NULL != data && 0 == size ) )
		return FUNC_RESULT_FAILED_ARGUMENT;

	snprintf( state.Memory.Buffer, IFACE_BUFFER_SIZE, ":%f %n", ( float )power, &currentLength );
	result = writeDown( state.Memory.Buffer, currentLength );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	snprintf( state.Memory.Buffer, IFACE_BUFFER_SIZE, "%llu %n", (long long unsigned)size, &currentLength );
	result = writeDown( state.Memory.Buffer, currentLength );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = writeDown( data, size );
	return result;
}

int transmitResponse( IfaceNeighbor_T * receiver, Crc_T crcInHeader, Crc_T crcFull ) {
	void			* responsePacket,
					* responsePayload;
	int				result;
	size_t			responseSize;
	IfaceHeader_T	* responseHeader;

	if( NULL == receiver )
		return FUNC_RESULT_FAILED_ARGUMENT;

	responseSize = IFACE_HEADER_SIZE + 2 * CRC_SIZE;
	responsePacket = malloc( responseSize );

	if( NULL == responsePacket )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	responseHeader = ( IfaceHeader_T * )responsePacket;
	responsePayload = responsePacket + IFACE_HEADER_SIZE;

	responseHeader->From = state.Config.Address;
	responseHeader->To = receiver->Address;
	responseHeader->Size = 2 * CRC_SIZE;
	responseHeader->CRC = 0; // that`s not implemented yet TODO
	responseHeader->TxPower = ( PowerInt_T )roundf( receiver->MinPower );
	responseHeader->Type = IfacePackType_IsResponse;

	memcpy( responsePayload, &crcInHeader, CRC_SIZE );
	memcpy( responsePayload + CRC_SIZE, &crcFull, CRC_SIZE );

	result = transmitAnyData( receiver->MinPower, responsePacket, responseSize );
	free( responsePacket );

	return result;
}

int transmitMessage( IfaceNeighbor_T * receiver, void * data, size_t size ) {
	void			* messagePacket,
					* messagePayload;
	int				result;
	size_t			messageSize;
	IfaceHeader_T	* messageHeader;

	if( NULL == receiver )
		return FUNC_RESULT_FAILED_ARGUMENT;

	messageSize = IFACE_HEADER_SIZE + size;
	messagePacket = malloc( messageSize );

	if( NULL == messagePacket )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	messageHeader = ( IfaceHeader_T * )messagePacket;
	messagePayload = messagePacket + IFACE_HEADER_SIZE;

	messageHeader->From = state.Config.Address;
	messageHeader->To = receiver->Address;
	messageHeader->Size = size;
	messageHeader->CRC = 0; // that`s not implemented yet TODO
	messageHeader->TxPower = ( PowerInt_T )roundf( receiver->MinPower );
	messageHeader->Type = IfacePackType_NeedResponse;

	memcpy( messagePayload, data, size );

	result = transmitAnyData( receiver->MinPower, messagePacket, messageSize );
	free( messagePacket );

	return result;
}

int transmitBeacon( void ) {
	void			* beaconPacket,
					* beaconPayload;
	int				result;
	size_t			beaconSize;
	IfaceHeader_T	* beaconHeader;
	IfaceFooter_T	* beaconFooter;

	beaconSize = IFACE_HEADER_SIZE + state.Config.BeaconPayloadSize + IFACE_FOOTER_SIZE;
	beaconPacket = malloc( beaconSize );

	if( NULL == beaconPacket )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	beaconHeader = ( IfaceHeader_T * )beaconPacket;
	beaconPayload = beaconPacket + IFACE_HEADER_SIZE;
	beaconFooter = ( IfaceFooter_T * )( beaconPayload + state.Config.BeaconPayloadSize );

	beaconHeader->From = state.Config.Address;
	memset( &( beaconHeader->To ), 0, IFACE_ADDR_SIZE );
	beaconHeader->Size = state.Config.BeaconPayloadSize;
	beaconHeader->CRC = 0; // that`s not implemented yet TODO
	beaconHeader->TxPower = ( PowerInt_T )roundf( state.Config.CurrentBeaconPower );
	beaconHeader->Type = IfacePackType_Beacon;

	memcpy( beaconPayload, state.Memory.BeaconPayload, state.Config.BeaconPayloadSize );

	beaconFooter->MinSensitivity = ( PowerInt_T )roundf( state.Config.CurrentSensitivity );

	result = transmitAnyData( beaconHeader->TxPower, beaconPacket, beaconSize );
	free( beaconPacket );

	return result;
}

PowerFloat_T calcMinPower( PowerInt_T startPower, PowerFloat_T finishPower ) {
	PowerFloat_T	neededPower = IFACE_MIN_FINISH_POWER + ( PowerFloat_T )startPower - finishPower;

	return ( IFACE_MAX_START_POWER < neededPower ? IFACE_MAX_START_POWER : neededPower );
}

int clearCommand( void ) {
	free( state.Memory.Command.MetaData );
	state.Memory.Command.MetaData = NULL;
	state.Memory.Command.MetaSize = 0;
	free( state.Memory.Command.Data );
	state.Memory.Command.Data = NULL;
	state.Memory.Command.DataSize = 0;
}

int processCommandIface( LayerCommandType_T commandType, void * metaData, void * data, size_t dataSize ) {
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

	clearCommand();
	state.Memory.Command.Command = commandType;
	state.Memory.Command.MetaSize = sizes[ commandType ];
	state.Memory.Command.MetaData = metaData;
	state.Memory.Command.DataSize = dataSize;
	state.Memory.Command.Data = data;

	return writeUp();
}

int processCommandIfaceRegister( void ) {
	IfaceRegisterMetadata_T	metadata;

	metadata.Length = IFACE_ADDR_SIZE;
	metadata.Value = state.Config.Address;

	return processCommandIface( LayerCommandType_RegisterInterface, &metadata, NULL, 0 );
}

int processCommandChannelRegisterResult( void ) {
	state.Config.IsConnectedToChannel = ( ( ChannelRegisterResultMetadata_T * ) state.Memory.Command.MetaData )->Registred;
	clearCommand();

	if( state.Config.IsConnectedToChannel ) {
		printf( "Interface registered in channel layer\n" );
		fflush( stdout );
		return FUNC_RESULT_SUCCESS;
	} else
		return processCommandIfaceRegister();
}

int processCommandIfaceUnknownDest( void ) {
	IfacePackStateMetadata_T	metadata;

	metadata.Id = state.Memory.ProcessingMessageId;
	metadata.State = IfacePackState_UnknownDest;

	return processCommandIface( LayerCommandType_MessageState, &metadata, NULL, 0 );
}

int processCommandIfaceTimeoutFinished( bool gotResponse ) {
	IfacePackStateMetadata_T	metadata;
	int 						result;

	metadata.Id = state.Memory.ProcessingMessageId;
	metadata.State = ( gotResponse ? IfacePackState_Responsed : IfacePackState_Timeouted );
	result = processCommandIface( LayerCommandType_MessageState, &metadata, NULL, 0 );
	state.Config.IsWaitingForResponse = false;

	if( IFACE_BEACON_INTERVAL > IFACE_RESPONSE_WAIT_INTERVAL )
		state.Config.BeaconIntervalCurrent = IFACE_BEACON_INTERVAL - IFACE_RESPONSE_WAIT_INTERVAL;

	return result;
}

int processCommandIfaceNeighborNew( IfaceAddr_T * address ) {
	IfaceNeighborMetadata_T	metadata;

	metadata.Neighbor = *address;

	return processCommandIface( LayerCommandType_NewNeighbor, &metadata, NULL, 0 );
}

int processCommandIfaceNeighborUpdate( IfaceAddr_T * address ) {
	IfaceNeighborMetadata_T	metadata;

	metadata.Neighbor = *address;

	return processCommandIface( LayerCommandType_UpdateNeighbor, &metadata, NULL, 0 );
}

int processReceivedBeacon( IfaceAddr_T * address, PowerFloat_T finishPower ) {
	int				result;
	IfaceNeighbor_T	* sender;
	PowerFloat_T	startPower;

	sender = neighborFind( address );
	startPower = calcMinPower( state.Memory.BufferFooter.MinSensitivity, finishPower );

	if( NULL != sender ) {
		result = neighborUpdate( sender, startPower );

		if( FUNC_RESULT_SUCCESS == result )
			result = processCommandIfaceNeighborUpdate( address );

	} else {
		result = neighborAdd( address, startPower );

		if( FUNC_RESULT_SUCCESS == result )
			result = processCommandIfaceNeighborNew( address );
	}

	return result;
}

int processMockitReceive( void ) {
	int				result;
	IfaceNeighbor_T	* sender;
	IfaceAddr_T		address;
	PowerFloat_T	power;

	result = receiveAnyData( &power );
	address = state.Memory.BufferHeader.From;

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	switch( state.Memory.BufferHeader.Type ) {
		case IfacePackType_NeedResponse :
			sender = neighborFind( &address );
			result = transmitResponse( sender, state.Memory.BufferHeader.CRC, 0 ); // crc is not implemented yet TODO
			break;

		case IfacePackType_IsResponse :
			// check what message response is for TODO
			result = processCommandIfaceTimeoutFinished( true );
			break;

		case IfacePackType_Beacon :
			result = processReceivedBeacon( &address, power );
			break;

		default :
			result = FUNC_RESULT_FAILED_ARGUMENT;
	}

//	if( FUNC_RESULT_SUCCESS == result && 0 < state.Memory.BufferHeader.Size ) // if contains payload
//		result = pushToChannel(); TODO push up received data ( processCommandIfaceReceived() or something similiar )

	return result;
}

int processMockitEvent( uint32_t events ) {
	if( events & EPOLLIN ) // if new data received
		return processMockitReceive();
	//other events TODO process lost connection
	return FUNC_RESULT_FAILED_ARGUMENT;
}

int processIfaceTransmit( IfaceNeighbor_T * receiver ) {
	int	result;

	if( NULL == receiver )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = transmitMessage( receiver, state.Memory.Command.Data, state.Memory.Command.DataSize );

	if( FUNC_RESULT_SUCCESS == result ) {
		state.Config.BeaconIntervalCurrent = IFACE_RESPONSE_WAIT_INTERVAL;
		state.Config.IsWaitingForResponse = true;
		clearCommand();
	}

	return result;
}

int processCommandChannelSend( void ) {
	IfaceNeighbor_T			* neighbor;
	ChannelSendMetadata_T	* metadata;
	int						result;

	metadata = ( ChannelSendMetadata_T * ) state.Memory.Command.MetaData;
	neighbor = neighborFind( &( metadata->To ) );
	state.Memory.ProcessingMessageId = metadata->Id;

	if( NULL == neighbor )
		result = processCommandIfaceUnknownDest();
	else
		result = processIfaceTransmit( neighbor );

	return result;
}

int processCommandChannelUpdateBeacon( void ) {
	if( ( NULL == state.Memory.Command.Data && 0 < state.Memory.Command.DataSize ) ||
		( NULL != state.Memory.Command.Data && 0 == state.Memory.Command.DataSize ) ||
		IFACE_MAX_PAYLOAD_BEACON_SIZE < state.Memory.Command.DataSize )
		return FUNC_RESULT_FAILED_ARGUMENT;

	memcpy( state.Memory.BeaconPayload, state.Memory.Command.Data, state.Memory.Command.DataSize );
	state.Config.BeaconPayloadSize = state.Memory.Command.DataSize;
	clearCommand();
	return FUNC_RESULT_SUCCESS;
}

int processChannelCommand( void ) {
	int	result;

	result = readUp();

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	if( state.Config.IsConnectedToChannel )
		switch( state.Memory.Command.Command ) {
			case LayerCommandType_Send :
				result = processCommandChannelSend();
				break;

			case LayerCommandType_UpdateBeaconPayload :
				result = processCommandChannelUpdateBeacon();
				break;

			default :
				result = FUNC_RESULT_FAILED_ARGUMENT;
				printf( "IFACE: unknown command %d from channel\n", state.Memory.Command.Command );
		}
	else if( LayerCommandType_RegisterInterfaceResult == state.Memory.Command.Command )
		result = processCommandChannelRegisterResult();

	return result;
}

int connectWithChannel( void ) {
	int	result;

	result = connectUp();

	if( FUNC_RESULT_SUCCESS == result )
		result = processCommandIfaceRegister();

	return result;
}

int connectWithChannelAgain( void ) {
	shutdown( state.Config.ChannelSocket, SHUT_RDWR );
	close( state.Config.ChannelSocket );
	state.Config.IsConnectedToChannel = false;
	return connectUp();
}

int processChannelEvent( uint32_t events ) {
	if( events & EPOLLIN ) // if new command from channel
		return processChannelCommand();
	else
		return connectWithChannelAgain();
}

void * MOAR_LAYER_ENTRY_POINT( void * arg ) {
	struct epoll_event	events[ IFACE_OPENING_SOCKETS ] = {{ 0 }},
						oneSocketEvent;
	int					result,
						epollHandler,
						eventsCount;

	if( NULL == arg )
		return NULL;

	strncpy( state.Config.ChannelSocketFilepath, ( ( MoarIfaceStartupParams_T * )arg )->socketToChannel, SOCKET_FILEPATH_SIZE );

	epollHandler = epoll_create( IFACE_OPENING_SOCKETS );
	oneSocketEvent.events = EPOLLIN | EPOLLET;
	result = preparePhysically();

	if( FUNC_RESULT_SUCCESS != result )
		return NULL;

	oneSocketEvent.data.fd = state.Config.MockitSocket;
	epoll_ctl( epollHandler, EPOLL_CTL_ADD, state.Config.MockitSocket, &oneSocketEvent );
	result = connectWithChannel();

	if( FUNC_RESULT_SUCCESS != result )
		return NULL;

	oneSocketEvent.data.fd = state.Config.ChannelSocket;
	epoll_ctl( epollHandler, EPOLL_CTL_ADD, state.Config.ChannelSocket, &oneSocketEvent );
	state.Config.BeaconIntervalCurrent = IFACE_BEACON_INTERVAL; // for cases when beaconIntervalCurrent will change

	while( true ) {
		eventsCount = epoll_wait( epollHandler, events, IFACE_OPENING_SOCKETS, state.Config.BeaconIntervalCurrent );
		result = FUNC_RESULT_SUCCESS;

		if( 0 == eventsCount ) {// timeout
			if( state.Config.IsWaitingForResponse )
				result = processCommandIfaceTimeoutFinished( false );
			else
				result = transmitBeacon();
		} else
			for( int eventIndex = 0; FUNC_RESULT_SUCCESS == result && eventIndex < eventsCount; eventIndex )
				if( events[ eventIndex ].data.fd == state.Config.MockitSocket )
					result = processMockitEvent( events[ eventIndex ].events );
				else if( events[ eventIndex ].data.fd == state.Config.ChannelSocket )
					result = processChannelEvent( events[ eventIndex ].events );
				else
					result = FUNC_RESULT_FAILED_ARGUMENT; // wrong socket

		if( FUNC_RESULT_SUCCESS != result )
			printf( "Error with %d code arised\n", result );

		fflush( stdout );
	}
}