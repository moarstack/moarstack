//
// Created by kryvashek on 13.08.16.
//

#include <moarIfacePhysicsRoutine.h>

int writeDown( IfaceState_T * layer, void * buffer, size_t bytes ) {
	int result = FUNC_RESULT_SUCCESS;

	if( ( NULL == buffer && 0 < bytes ) || 0 >= layer->Config.MockitSocket )
		result = FUNC_RESULT_FAILED_ARGUMENT;

	if( FUNC_RESULT_SUCCESS == result ) {
		for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
			result = ( int )write( layer->Config.MockitSocket, buffer, bytes );

			if( result == bytes )
				break;
			else if( 1 < IFACE_SEND_ATTEMPTS_COUNT )
				sleep( IFACE_MOCKIT_WAIT_INTERVAL );
		}

		result = ( bytes == result ? FUNC_RESULT_SUCCESS : FUNC_RESULT_FAILED_IO );
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "writing into the mockit socket" );

	return result;
}

int readDown( IfaceState_T * layer, void * buffer, size_t bytes ) {
	int	result = FUNC_RESULT_SUCCESS;

	if( ( NULL == buffer && 0 < bytes ) || 0 >= layer->Config.MockitSocket ) {
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, FUNC_RESULT_FAILED_ARGUMENT, "reading from the mockit socket" );
		result = FUNC_RESULT_FAILED_ARGUMENT;
	}

	if( FUNC_RESULT_SUCCESS == result )
		for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
			result = ( int )read( layer->Config.MockitSocket, buffer, bytes );

			if( 0 < result )
				break;
			else if( 1 < IFACE_SEND_ATTEMPTS_COUNT )
				sleep( IFACE_MOCKIT_WAIT_INTERVAL );
		}

	if( 0 >= result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, FUNC_RESULT_FAILED_IO, "reading from the mockit socket" );

	return result;
}

int connectDown( IfaceState_T * layer ) {
	int	result;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = SocketOpenFile( IFACE_MOCKIT_SOCKET_FILE, false, &( layer->Config.MockitSocket ) );

		if( FUNC_RESULT_SUCCESS == result )
			break;
		else if( 1 < IFACE_SEND_ATTEMPTS_COUNT )
			sleep( IFACE_MOCKIT_WAIT_INTERVAL );
	}

	return result;
}
