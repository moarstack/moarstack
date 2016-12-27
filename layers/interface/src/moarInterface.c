//
// Created by svalov, kryvashek on 05.07.16.
//

#include <moarInterfacePrivate.h>
#include <moarIfaceNeighborsRoutine.h>
#include <moarIfaceTransmitReceive.h>
#include <moarIfaceCommands.h>
#include <moarIfaceMockitActions.h>
#include <moarCommonSettings.h>
#include <mockIfaceSettings.h>

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

int setStrValue( char * variable, const char * source, const char * defaultValue, size_t size ) {
	if( NULL == variable || ( NULL == source && NULL == defaultValue ) )
		return FUNC_RESULT_FAILED_ARGUMENT;

	strncpy( variable, ( NULL == source ? defaultValue : source ), size );
	return FUNC_RESULT_SUCCESS;
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
	CHECK_RESULT( bindingBindStructFunc(paramsStruct->LayerConfig, makeIfaceSockBinding, &socketInfo) );

	mockIface ifaceSettings = {0};
	CHECK_RESULT( bindingBindStructFunc(paramsStruct->LayerConfig, makeMockIfaceBinding, &ifaceSettings) );

	CHECK_RESULT( setStrValue( layer->Config.ChannelSocketFilepath, socketInfo.FileName, DEFAULT_CHANNEL_SOCKET_FILE, SOCKET_FILEPATH_SIZE ) );
	CHECK_RESULT( setStrValue( layer->Config.MockitSocketFilepath, ifaceSettings.MockItSocket, DEFAULT_MOCKIT_SOCKET_FILE, SOCKET_FILEPATH_SIZE ) );
	CHECK_RESULT( setStrValue( layer->Config.LogFilepath, ifaceSettings.LogPath, DEFAULT_MOCKIFACE_LOG_FILE, LOG_FILEPATH_SIZE ) );
	layer->Config.Address = ifaceSettings.Address;

	result = LogOpen( layer->Config.LogFilepath, &( layer->Config.LogHandle) );

	LogSetLevelLog( layer->Config.LogHandle, LogLevel_DebugQuiet );
	LogSetLevelDump( layer->Config.LogHandle, LogLevel_DebugQuiet );

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
	moarTime_T			oldStart,
						newStart,
						now;
	moarTimeInterval_T	interval;

	if( NULL == layer )
		return FUNC_RESULT_FAILED_ARGUMENT; // what is negative timeout? Jump to the past???

	oldStart = ( layer->Config.IsWaitingForResponse ? layer->Memory.LastNeedResponse : layer->Memory.LastBeacon );
	interval = ( layer->Config.IsWaitingForResponse ? IFACE_RESPONSE_WAIT_INTERVAL : IFACE_BEACON_INTERVAL );
	newStart = timeAddInterval( oldStart, interval );
	now = timeGetCurrent();

	return ( int )( 1 == timeCompare( newStart, now ) ? timeGetDifference( newStart, now ) : 0 );
}

void * MOAR_LAYER_ENTRY_POINT( void * arg ) {
	IfaceState_T		state = { 0 };
	int					result,
						eventsCount,
						timeout,
						count = 0,
						start;

	result = initInterface( &state, arg );

	if( FUNC_RESULT_SUCCESS != result )
		return NULL;

	while( state.Config.IsRunning ) {
		result = neighborClean( &state );

		if( FUNC_RESULT_SUCCESS != result )
			LogErrMoar( state.Config.LogHandle, LogLevel_Warning, result, "failed cleaning neighbors table" );

		timeout = ifaceEpollTimeout( &state );
		LogWrite( state.Config.LogHandle, LogLevel_Dump, "%d cycle iteration, will (or not) wait in epoll for %d msecs", ++count, timeout );

		if( 0 < timeout ) {
			start = timeGetCurrent();
			eventsCount = epoll_wait( state.Config.EpollHandler, state.Memory.EpollEvents, IFACE_OPENING_SOCKETS, timeout );
			LogWrite( state.Config.LogHandle, LogLevel_Dump, "%d cycle iteration epoll finished in %d msecs", count, timeGetDifference( timeGetCurrent(), start ) );
		} else
			eventsCount = 0;

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