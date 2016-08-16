//
// Created by kryvashek on 13.08.16.
//

#include <moarIfacePhysicsRoutine.h>

int writeDown( IfaceState_T * layer, void * buffer, size_t bytes ) {
	ssize_t result;

	if( ( NULL == buffer && 0 < bytes ) ||
		FUNC_RESULT_SUCCESS >= layer->Config.MockitSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = write( layer->Config.MockitSocket, buffer, bytes );

		if( result == bytes )
			return FUNC_RESULT_SUCCESS;
		else if( 1 < IFACE_SEND_ATTEMPTS_COUNT )
			sleep( IFACE_MOCKIT_WAIT_INTERVAL );
	}

	return FUNC_RESULT_FAILED_IO;
}

ssize_t readDown( IfaceState_T * layer, void * buffer, size_t bytes ) {
	ssize_t	result;

	if( ( NULL == buffer && 0 < bytes ) ||
		FUNC_RESULT_SUCCESS >= layer->Config.MockitSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( int attempt = 0; attempt < IFACE_SEND_ATTEMPTS_COUNT; attempt++ ) {
		result = read( layer->Config.MockitSocket, buffer, bytes );

		if( 0 < result )
			break;
		else if( 1 < IFACE_SEND_ATTEMPTS_COUNT )
			sleep( IFACE_MOCKIT_WAIT_INTERVAL );
	}

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
