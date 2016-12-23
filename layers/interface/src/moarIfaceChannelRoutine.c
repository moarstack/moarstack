//
// Created by kryvashek on 13.08.16.
//

#include <moarIfaceChannelRoutine.h>
#include <moarInterfacePrivate.h>

int writeUp( IfaceState_T * layer, LayerCommandStruct_T * command ) {
	int result;

	if( NULL == layer || NULL == command )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_PUSH_ATTEMPTS_COUNT; attempt++ ) {
		result = WriteCommand( layer->Config.ChannelSocket, command );

		if( FUNC_RESULT_SUCCESS == result )
			break;
		else if( 1 < IFACE_PUSH_ATTEMPTS_COUNT )
			sleep( IFACE_CHANNEL_WAIT_INTERVAL );
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "writing into the channel socket" );

	return result;
}

int readUp( IfaceState_T * layer, LayerCommandStruct_T * command ) {
	int result;

	if( NULL == layer || NULL == command )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_PUSH_ATTEMPTS_COUNT; attempt++ ) {
		result = ReadCommand( layer->Config.ChannelSocket, command );

		if( FUNC_RESULT_SUCCESS == result )
			break;
		else if( 1 < IFACE_PUSH_ATTEMPTS_COUNT )
			sleep( IFACE_CHANNEL_WAIT_INTERVAL );
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "reading from the channel socket" );

	return result;
}

int connectUp( IfaceState_T * layer ) {
	int	result = FUNC_RESULT_FAILED;

	for( int attempt = 0; attempt < IFACE_PUSH_ATTEMPTS_COUNT; attempt++ ) {
		result = SocketOpenFile( layer->Config.ChannelSocketFilepath, false, &( layer->Config.ChannelSocket ) );

		if( FUNC_RESULT_SUCCESS == result )
			break;
		else if( 1 < IFACE_PUSH_ATTEMPTS_COUNT )
			sleep( IFACE_CHANNEL_WAIT_INTERVAL );
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "opening the channel socket file" );

	return result;
}