//
// Created by svalov, kryvashek on 05.07.16.
//

#include <moarInterfacePrivate.h>
#include <moarIfacePhysicsRoutine.h>
#include <moarIfaceChannelRoutine.h>
#include <moarIfaceNeighborsRoutine.h>
#include <moarIfaceTransmitReceive.h>
#include <moarIfaceCommands.h>
#include <moarIfaceMockitActions.h>

int actMockitEvent( IfaceState_T * layer, uint32_t events ) {
	int result;

	if( events & EPOLLIN ) // if new data received
		result = actMockitReceived( layer );
	else
		result = actMockitReconnection( layer );

	return result;
}

int processChannelEvent( IfaceState_T * layer, uint32_t events ) {
	int	result;

	if( events & EPOLLIN ) // if new command from channel
		result = processCommandChannel( layer );
	else
		result = processChannelReconnection( layer );

	return result;
}

void * MOAR_LAYER_ENTRY_POINT( void * arg ) {
	IfaceState_T		state = { 0 };
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
	result = actMockitConnection( &state );

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
					result = actMockitEvent( &state, events[ eventIndex ].events );
				else if( events[ eventIndex ].data.fd == state.Config.ChannelSocket )
					result = processChannelEvent( &state, events[ eventIndex ].events );
				else
					result = FUNC_RESULT_FAILED_ARGUMENT; // wrong socket

				if( FUNC_RESULT_SUCCESS != result ) {
					printf( "IFACE: Error with %d code arised\n", result );
					fflush( stdout );
				}
			}

	}
}