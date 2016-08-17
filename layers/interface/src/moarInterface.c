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

	if( events & EPOLLHUP )
		result = actMockitReconnection( layer, false );
	else if( events & EPOLLIN )// if new data received
		result = actMockitReceived( layer );
	else if ( events & EPOLLERR )
		result = actMockitReconnection( layer, true );
	else
		result = FUNC_RESULT_FAILED_ARGUMENT;

	return result;
}

int processChannelEvent( IfaceState_T * layer, uint32_t events ) {
	int	result;

	if( events & EPOLLHUP )
		result = processChannelReconnection( layer, false );
	else if( events & EPOLLIN )// if new command from channel
		result = processCommandChannel( layer );
	else if ( events & EPOLLERR )
		result = processChannelReconnection( layer, true );
	else
		result = FUNC_RESULT_FAILED_ARGUMENT;

	return result;
}

int initInterface( IfaceState_T * layer, void * params ) {
	int							result;
	MoarIfaceStartupParams_T	* paramsStruct;

	if( NULL == params )
		return FUNC_RESULT_FAILED_ARGUMENT;

	paramsStruct = ( MoarIfaceStartupParams_T * )params;
	strncpy( layer->Config.ChannelSocketFilepath, paramsStruct->socketToChannel, SOCKET_FILEPATH_SIZE );
	layer->Config.EpollHandler = epoll_create( IFACE_OPENING_SOCKETS );
	result = actMockitConnection( layer );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = processChannelConnection( layer );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	layer->Config.BeaconIntervalCurrent = IFACE_BEACON_INTERVAL; // for cases when beaconIntervalCurrent will change

	return FUNC_RESULT_SUCCESS;
}

void * MOAR_LAYER_ENTRY_POINT( void * arg ) {
	IfaceState_T		state = { 0 };
	struct epoll_event	* event;
	int					result,
						eventsCount;

	result = initInterface( &state, arg );

	if( FUNC_RESULT_SUCCESS != result ) {
		printf( "IFACE: error %d while init\n", result );
		return NULL;
	}

	while( true ) {
		eventsCount = epoll_wait( state.Config.EpollHandler, state.Memory.EpollEvents, IFACE_OPENING_SOCKETS, state.Config.BeaconIntervalCurrent );

		if( 0 == eventsCount ) {// timeout
			if( state.Config.IsWaitingForResponse )
				result = processCommandIfaceTimeoutFinished( &state, false );
			else
				result = transmitBeacon( &state );
		} else
			for( int eventIndex = 0; eventIndex < eventsCount; eventIndex++ ) {
				if( state.Memory.EpollEvents[ eventIndex ].data.fd == state.Config.MockitSocket )
					result = actMockitEvent( &state, state.Memory.EpollEvents[ eventIndex ].events );
				else if( state.Memory.EpollEvents[ eventIndex ].data.fd == state.Config.ChannelSocket )
					result = processChannelEvent( &state, state.Memory.EpollEvents[ eventIndex ].events );
				else
					result = FUNC_RESULT_FAILED_ARGUMENT; // wrong socket

				if( FUNC_RESULT_SUCCESS != result ) {
					printf( "IFACE: Error with %d code arised\n", result );
					fflush( stdout );
				}
			}

	}
}