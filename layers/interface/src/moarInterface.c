//
// Created by svalov, kryvashek on 05.07.16.
//

#include <moarInterfacePrivate.h>

static IfaceState_T	state = { 0 };

int writeDown( void * buffer, size_t bytes ) {
	ssize_t result = -1;

	if( ( NULL == buffer && 0 < bytes ) ||
		FUNC_RESULT_SUCCESS >= state.Config.MockitSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = write( state.Config.MockitSocket, buffer, bytes );

		if( result == bytes )
			return FUNC_RESULT_SUCCESS;
		else
			sleep( IFACE_MOCKIT_WAIT_INTERVAL );
	}

	return FUNC_RESULT_FAILED_IO;
}

int readDown( void * buffer, size_t bytes ) {
	ssize_t	result = -1;

	if( ( NULL == buffer && 0 < bytes ) ||
		FUNC_RESULT_SUCCESS >= state.Config.MockitSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = read( state.Config.MockitSocket, buffer, bytes );

		if( 0 < result )
			return FUNC_RESULT_SUCCESS;
		else
			sleep( IFACE_MOCKIT_WAIT_INTERVAL );
	}

	return result;
}

int connectDown( int * sock ) {
	int	result = FUNC_RESULT_FAILED;

	if( NULL == sock )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT && result != FUNC_RESULT_SUCCESS; attempt++ )
		result = SocketOpenFile( IFACE_MOCKIT_SOCKET_FILE, false, sock );

	return result;
}

int preparePhysically( void ) {
	struct timeval	moment;
	int				result,
					length,
					address;

	result = connectDown( &( state.Config.MockitSocket ) );

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

		if( FUNC_RESULT_SUCCESS != result )
			return FUNC_RESULT_FAILED_IO;
	} while( 0 != strncmp( IFACE_REGISTRATION_OK, state.Memory.Buffer, strlen( IFACE_REGISTRATION_OK ) ) );

	memcpy( &( state.Config.Address ), &address, IFACE_ADDR_SIZE );
	printf( "Interface registered in MockIT with address %d\n", address );
	fflush( stdout );

	return FUNC_RESULT_SUCCESS;
}

int writeUp( LayerCommandStruct_T * command ) {
	int result;

	if( NULL == command )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_PUSH_ATTEMPTS_COUNT; attempt++ ) {
		result = WriteCommand( state.Config.ChannelSocket, command );

		if( FUNC_RESULT_SUCCESS == result )
			return FUNC_RESULT_SUCCESS;
		else
			sleep( IFACE_CHANNEL_WAIT_INTERVAL );
	}

	return FUNC_RESULT_FAILED_IO;
}

int readUp( LayerCommandStruct_T * command ) {
	int result;

	if( NULL == command )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_PUSH_ATTEMPTS_COUNT; attempt++ ) {
		result = ReadCommand( state.Config.ChannelSocket, command );

		if( FUNC_RESULT_SUCCESS == result )
			return FUNC_RESULT_SUCCESS;
		else
			sleep( IFACE_CHANNEL_WAIT_INTERVAL );
	}

	return FUNC_RESULT_FAILED_IO;
}

int connectUp( SocketFilepath_T channelSocketFile ) {
	int	result = FUNC_RESULT_FAILED;

	if( NULL == channelSocketFile )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT && result != FUNC_RESULT_SUCCESS; attempt++ )
		result = SocketOpenFile( channelSocketFile, false, &( state.Config.ChannelSocket ) );

	return result;
}

int connectWithChannel( SocketFilepath_T filepath ) {
	LayerCommandStruct_T	command;
	IfaceRegisterMetadata_T	plainAddr;
	int						result;
	bool					completed = false;

	if( NULL == filepath )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = connectUp( filepath );

	if( FUNC_RESULT_SUCCESS != result )
		return FUNC_RESULT_FAILED;

	plainAddr.Length = IFACE_ADDR_SIZE;
	plainAddr.Value = state.Config.Address;

	do {
		command.Command = LayerCommandType_RegisterInterface;
		command.MetaSize = IFACE_REGISTER_METADATA_SIZE;
		command.MetaData = &plainAddr;
		command.DataSize = 0;
		command.Data = NULL;
		result = writeUp( &command );

		if( FUNC_RESULT_SUCCESS != result )
			return FUNC_RESULT_FAILED_IO;

		result = readUp( &command );

		if( FUNC_RESULT_SUCCESS != result )
			return FUNC_RESULT_FAILED_IO;

		if( LayerCommandType_RegisterInterfaceResult == command.Command )
			completed = ( ( ChannelRegisterResultMetadata_T * ) command.MetaData )->Registred;

	} while( !completed );

	printf( "Interface registered in channel layer\n" );
	fflush( stdout );

	return FUNC_RESULT_SUCCESS;
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
		bytesDone += read( state.Config.MockitSocket, ( char * )destination + bytesDone, expectedSize - bytesDone );

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

	bytesLeft = read( state.Config.MockitSocket, buffer, IFACE_BUFFER_SIZE );

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
	int		bytesWritten = 0,
			bytesShouldWrite = 0,
			currentLength;

	if( ( NULL == data && 0 < size ) || ( NULL != data && 0 == size ) )
		return FUNC_RESULT_FAILED_ARGUMENT;

	snprintf( state.Memory.Buffer, IFACE_BUFFER_SIZE, ":%f %n", ( float )power, &currentLength );
	bytesShouldWrite += currentLength;
	bytesWritten += write( state.Config.MockitSocket, state.Memory.Buffer, currentLength );

	snprintf( state.Memory.Buffer, IFACE_BUFFER_SIZE, "%llu %n", size, &currentLength );
	bytesShouldWrite += currentLength;
	bytesWritten += write( state.Config.MockitSocket, state.Memory.Buffer, currentLength );

	bytesShouldWrite += size;
	bytesWritten = write( state.Config.MockitSocket, data, size );

	if( bytesWritten < bytesShouldWrite )
		return FUNC_RESULT_FAILED_IO;

	return FUNC_RESULT_SUCCESS;
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

PowerFloat_T calcMinPower( PowerInt_T startPower, PowerFloat_T finishPower ) {
	PowerFloat_T	neededPower = IFACE_MIN_FINISH_POWER + ( PowerFloat_T )startPower - finishPower;

	return ( IFACE_MAX_START_POWER < neededPower ? IFACE_MAX_START_POWER : neededPower );
}

int pushToChannel( void ) {
	return FUNC_RESULT_SUCCESS;
}

int processMockitReceive( void ) {
	int				result;
	IfaceNeighbor_T	* sender;
	IfaceAddr_T		address;
	PowerFloat_T	startPower,
					finishPower;

	result = receiveAnyData( &finishPower );
	address = state.Memory.BufferHeader.From;

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	switch( state.Memory.BufferHeader.Type ) {
		case IfacePackType_NeedResponse :
			sender = neighborFind( &address );
			result = transmitResponse( sender, state.Memory.BufferHeader.CRC, 0 ); // crc is not implemented yet TODO
			break;

		case IfacePackType_IsResponse :
			// update current message state TODO
			// drop message - nothing to do here due to storing got response in preallocated memory
			break;

		case IfacePackType_Beacon :
			sender = neighborFind( &address );
			startPower = calcMinPower( state.Memory.BufferFooter.MinSensitivity, finishPower );

			if( NULL != sender )
				result = neighborUpdate( sender, startPower );
			else {
				result = neighborAdd( &address, startPower );
				// send to channel new neighbor command TODO
			}
			break;

		default :
			result = FUNC_RESULT_FAILED_ARGUMENT;
	}

	if( FUNC_RESULT_SUCCESS == result && 0 < state.Memory.BufferHeader.Size ) // if contains payload
		result = pushToChannel();

	return result;
}

int processMockitEvent( uint32_t events ) {
	if( events & EPOLLIN ) // if new data received
		return processMockitReceive();
	//other events
	return FUNC_RESULT_FAILED_ARGUMENT;
}

void * MOAR_LAYER_ENTRY_POINT( void * arg ) {
	struct epoll_event	events[ IFACE_OPENING_SOCKETS ] = {{ 0 }},
						oneSocketEvent;
	int					result,
						epollHandler,
						eventsCount;

	if( NULL == arg )
		return NULL;

	// load configuration

	epollHandler = epoll_create( IFACE_OPENING_SOCKETS );
	oneSocketEvent.events = EPOLLIN | EPOLLET;
	result = preparePhysically();

//	if( FUNC_RESULT_SUCCESS != result )
//		return NULL;

	oneSocketEvent.data.fd = state.Config.MockitSocket;
	epoll_ctl( epollHandler, EPOLL_CTL_ADD, state.Config.MockitSocket, &oneSocketEvent );
	result = connectWithChannel( ( ( MoarIfaceStartupParams_T * )arg )->socketToChannel );

	if( FUNC_RESULT_SUCCESS != result )
		return NULL;

	oneSocketEvent.data.fd = state.Config.ChannelSocket;
	epoll_ctl( epollHandler, EPOLL_CTL_ADD, state.Config.ChannelSocket, &oneSocketEvent );
	state.Config.BeaconIntervalCurrent = IFACE_BEACON_INTERVAL; // for cases when beaconIntervalCurrent will change

	while( true ) {
		eventsCount = epoll_wait( epollHandler, events, IFACE_OPENING_SOCKETS, state.Config.BeaconIntervalCurrent );
		result = FUNC_RESULT_SUCCESS;

		if( 0 == eventsCount ) {// timeout
			// is timeout caused by no response during specified period?
			// if yes
				// send bad message state to channel
			// else
				// transmit beacon
		} else
			for( int eventIndex = 0; FUNC_RESULT_SUCCESS == result && eventIndex < eventsCount; eventIndex )
				if( events[ eventIndex ].data.fd == state.Config.MockitSocket )
					result = processMockitEvent( events[ eventIndex ].events );
				else if( events[ eventIndex ].data.fd == state.Config.ChannelSocket ) {
					// process channel event
				}
				else
					result = FUNC_RESULT_FAILED_ARGUMENT; // wrong socket

		if( FUNC_RESULT_SUCCESS != result )
			printf( "Error with %d code arised\n", result );
	}
}