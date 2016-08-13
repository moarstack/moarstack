//
// Created by svalov, kryvashek on 05.07.16.
//

#include <moarInterfacePrivate.h>
#include <moarIfacePhysicsRoutine.h>
#include <moarIfaceChannelRoutine.h>
#include <moarIfaceNeighborsRoutine.h>
#include <moarIfaceTransmitReceive.h>
#include <moarIfaceCommands.h>

static IfaceState_T	state = { 0 };

PowerFloat_T calcMinPower( PowerInt_T startPower, PowerFloat_T finishPower ) {
	PowerFloat_T	neededPower = IFACE_MIN_FINISH_POWER + ( PowerFloat_T )startPower - finishPower;

	return ( IFACE_MAX_START_POWER < neededPower ? IFACE_MAX_START_POWER : neededPower );
}

int processReceivedBeacon( IfaceAddr_T * address, PowerFloat_T finishPower ) {
	int				result;
	IfaceNeighbor_T	* sender;
	PowerFloat_T	startPower;

	sender = neighborFind( &state, address );
	startPower = calcMinPower( state.Memory.BufferFooter.MinSensitivity, finishPower );

	if( NULL != sender ) {
		result = neighborUpdate( &state, sender, startPower );

		if( FUNC_RESULT_SUCCESS == result )
			result = processCommandIfaceNeighborUpdate( &state, address );

	} else {
		result = neighborAdd( &state, address, startPower );

		if( FUNC_RESULT_SUCCESS == result )
			result = processCommandIfaceNeighborNew( &state, address );
	}

	return result;
}

int processMockitRegister( void ) {
	struct timeval	moment;
	int				result,
					length,
					address;

	gettimeofday( &moment, NULL );
	srand( ( unsigned int )( moment.tv_usec ) );
	address = 1 + rand() % IFACE_ADDRESS_LIMIT;
	snprintf( state.Memory.Buffer, IFACE_BUFFER_SIZE, "%d%n", address, &length );
	memcpy( &( state.Config.Address ), &address, IFACE_ADDR_SIZE );
	result = writeDown( &state, state.Memory.Buffer, length );

	return result;
}

int processMockitRegisterResult( void ) {
	int	result;

	result = readDown( &state, state.Memory.Buffer, IFACE_BUFFER_SIZE );

	if( 0 >= result )
		return FUNC_RESULT_FAILED_IO;

	state.Config.IsConnectedToMockit = ( 0 == strncmp( IFACE_REGISTRATION_OK, state.Memory.Buffer, IFACE_REGISTRATION_OK_SIZE ) );

	if( state.Config.IsConnectedToMockit ) {
		printf( "Interface registered in MockIT with address %08X\n", *( unsigned int * )&( state.Config.Address ) );
		fflush( stdout );
		return FUNC_RESULT_SUCCESS;
	} else
		return processMockitRegister();
}

int processReceived( void ) {
	int				result;
	IfaceNeighbor_T	* sender;
	IfaceAddr_T		address;
	PowerFloat_T	power;

	if( state.Config.IsConnectedToMockit ) {
		result = receiveAnyData( &state, &power );
		address = state.Memory.BufferHeader.From;

		if( FUNC_RESULT_SUCCESS == result )
			switch( state.Memory.BufferHeader.Type ) {
				case IfacePackType_NeedNoResponse:
					break;

				case IfacePackType_NeedResponse :
					sender = neighborFind( &state, &address );
					result = transmitResponse( &state, sender, state.Memory.BufferHeader.CRC, 0 ); // crc is not implemented yet TODO
					break;

				case IfacePackType_IsResponse :
					// check what message response is for TODO
					result = processCommandIfaceTimeoutFinished( &state, true );
					break;

				case IfacePackType_Beacon :
					result = processReceivedBeacon( &address, power );
					break;

				default :
					result = FUNC_RESULT_FAILED_ARGUMENT;
			}

		if( FUNC_RESULT_SUCCESS == result &&
			IfacePackType_IsResponse != state.Memory.BufferHeader.Type &&
			0 < state.Memory.BufferHeader.Size ) // if contains payload
			result = processCommandIfaceReceived( &state );

	} else
		result = processMockitRegisterResult();

	return result;
}

int connectWithMockit( void ) {
	int	result;

	result = connectDown( &state );

	if( FUNC_RESULT_SUCCESS == result )
		result = processMockitRegister();

	return result;
}

int connectWithMockitAgain( void ) {
	shutdown( state.Config.MockitSocket, SHUT_RDWR );
	close( state.Config.MockitSocket );
	state.Config.IsConnectedToMockit = false;
	return connectDown( &state );
}

int processMockitEvent( uint32_t events ) {
	int result;

	if( events & EPOLLIN ) // if new data received
		result = processReceived();
	else
		result = connectWithMockitAgain();

	return result;
}

int processChannelEvent( uint32_t events ) {
	int	result;

	if( events & EPOLLIN ) // if new command from channel
		result = processCommandChannel( &state );
	else
		result = processChannelReconnection( &state );

	return result;
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
	result = connectWithMockit();

	if( FUNC_RESULT_SUCCESS != result )
		return NULL;

	oneSocketEvent.data.fd = state.Config.MockitSocket;
	epoll_ctl( epollHandler, EPOLL_CTL_ADD, state.Config.MockitSocket, &oneSocketEvent );
	result = processChannelConnection( &state );

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
				result = processCommandIfaceTimeoutFinished( &state, false );
			else
				result = transmitBeacon( &state );
		} else
			for( int eventIndex = 0; eventIndex < eventsCount; eventIndex++ ) {
				if( events[ eventIndex ].data.fd == state.Config.MockitSocket )
					result = processMockitEvent( events[ eventIndex ].events );
				else if( events[ eventIndex ].data.fd == state.Config.ChannelSocket )
					result = processChannelEvent( events[ eventIndex ].events );
				else
					result = FUNC_RESULT_FAILED_ARGUMENT; // wrong socket

				if( FUNC_RESULT_SUCCESS != result ) {
					printf( "IFACE: Error with %d code arised\n", result );
					fflush( stdout );
				}
			}

	}
}