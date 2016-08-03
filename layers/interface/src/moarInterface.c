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

int processMockitReceive( void ) {
	return FUNC_RESULT_SUCCESS;
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

	if( FUNC_RESULT_SUCCESS != result )
		return NULL;

	oneSocketEvent.data.fd = state.Config.MockitSocket;
	epoll_ctl( epollHandler, EPOLL_CTL_ADD, state.Config.MockitSocket, &oneSocketEvent );

    // connect to channel layer
    // send connect command
    // wait for connected answer

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