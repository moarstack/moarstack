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
#include <moarInterface.h>
#include <moarCommonSettings.h>
#include <moarLayerEntryPoint.h>
#include <mockIfaceSettings.h>
#include <moarTime.h>
#include "mockIfaceSettings.h"

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
	MoarLayerStartupParams_T	* paramsStruct;

	if( NULL == layer || NULL == params )
		return FUNC_RESULT_FAILED_ARGUMENT;

	memset( layer, 0, sizeof( IfaceState_T ) );
	layer->Memory.LastBeacon = timeGetCurrent();
	paramsStruct = ( MoarLayerStartupParams_T * )params;

	ifaceSocket socketInfo = {0};

	int res = bindingBindStructFunc(paramsStruct->LayerConfig, makeIfaceSockBinding, &socketInfo);
	CHECK_RESULT(res);

	mockIface ifaceSettings = {0};
	res = bindingBindStructFunc(paramsStruct->LayerConfig, makeMockIfaceBinding, &ifaceSettings);
	CHECK_RESULT(res);

	strncpy( layer->Config.ChannelSocketFilepath, socketInfo.FileName, SOCKET_FILEPATH_SIZE );
	strncpy( layer->Config.MockitSocketFilepath, ifaceSettings.MockItSocket, SOCKET_FILEPATH_SIZE );
	strncpy( layer->Config.LogFilepath, ifaceSettings.LogPath, LOG_FILEPATH_SIZE );
	layer->Config.Address = ifaceSettings.Address;

	result = LogOpen( layer->Config.LogFilepath, &( layer->Config.LogHandle) );

	if( FUNC_RESULT_SUCCESS == result )
		result = LogWrite( layer->Config.LogHandle, LogLevel_Information, "interface starting with logging into %.*s\n", LOG_FILEPATH_SIZE, layer->Config.LogFilepath );

	if( FUNC_RESULT_SUCCESS == result )
		layer->Config.EpollHandler = epoll_create( IFACE_OPENING_SOCKETS );

	if( -1 != layer->Config.EpollHandler )
		result = actMockitConnection( layer );
	else
		result = FUNC_RESULT_FAILED;

	if( FUNC_RESULT_SUCCESS == result )
		result = processChannelConnection( layer );

	if( FUNC_RESULT_SUCCESS == result ) {
		layer->Config.WaitIntervalCurrent = IFACE_BEACON_INTERVAL; // for cases when WaitIntervalCurrent will change
		layer->Config.IsRunning = true;
	}

	if( FUNC_RESULT_SUCCESS != result ) {
		if( 0 == layer->Config.LogHandle )
			fprintf( stderr, "IFACE: error %d while init\n", result );
		else {
			LogErrMoar( layer->Config.LogHandle, LogLevel_Critical, result, "init" );
			LogClose( &( layer->Config.LogHandle ) );
		}
	}

	return result;
}

int ifaceEpollTimeout( IfaceState_T * layer ) {
	moarTime_T			start;
	moarTimeInterval_T	interval;

	if( NULL == layer )
		return FUNC_RESULT_FAILED_ARGUMENT; // what is negative timeout? Jump to the past???

	start = ( layer->Config.IsWaitingForResponse ? layer->Memory.LastNeedResponse : layer->Memory.LastBeacon );
	interval = ( layer->Config.IsWaitingForResponse ? IFACE_RESPONSE_WAIT_INTERVAL : IFACE_BEACON_INTERVAL );
	return ( int )timeGetDifference( timeAddInterval( start, interval ), timeGetCurrent() );
}

void * MOAR_LAYER_ENTRY_POINT( void * arg ) {
	IfaceState_T		state = { 0 };
	int					result,
						eventsCount;

	result = initInterface( &state, arg );

	if( FUNC_RESULT_SUCCESS != result )
		return NULL;

	while( state.Config.IsRunning ) {
		eventsCount = epoll_wait( state.Config.EpollHandler, state.Memory.EpollEvents, IFACE_OPENING_SOCKETS, ifaceEpollTimeout( &state ) );

		if( 0 == eventsCount ) {// timeout
			if( state.Config.IsWaitingForResponse )
				result = processCommandIfaceTimeoutFinished( &state, false );
			else
				result = transmitBeacon( &state );

			if( FUNC_RESULT_SUCCESS != result )
				LogErrMoar( state.Config.LogHandle, LogLevel_Error, result, "main cycle" );
		} else
			for( int eventIndex = 0; eventIndex < eventsCount; eventIndex++ ) {
				if( state.Memory.EpollEvents[ eventIndex ].data.fd == state.Config.MockitSocket )
					result = actMockitEvent( &state, state.Memory.EpollEvents[ eventIndex ].events );
				else if( state.Memory.EpollEvents[ eventIndex ].data.fd == state.Config.ChannelSocket )
					result = processChannelEvent( &state, state.Memory.EpollEvents[ eventIndex ].events );
				else
					result = FUNC_RESULT_FAILED_ARGUMENT; // wrong socket

				if( FUNC_RESULT_SUCCESS != result )
					LogErrMoar( state.Config.LogHandle, LogLevel_Error, result, "main cycle" );
			}

	}

}