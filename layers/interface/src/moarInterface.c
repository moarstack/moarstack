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

void * MOAR_LAYER_ENTRY_POINT( void * arg ) {
	struct epoll_event	events[ IFACE_OPENING_SOCKETS ] = {{ 0 }},
						oneSocketEvent;
	int					result,
						epollHandler;

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
    // in poll
        // if timeout
            // if time to send beacon | interface ready
                // send beacon

        // if signal event
            // ?????

        // if transmission done
            // switch interface to listen
            // update current message state

        // if new data received
            // if correct crc
                // if data need response
                    // send response
                // if data is response
                    // update current message state
                    // drop message
                // if data is beacon
                    // if neighbor found
                        //update neighbor
                    // else
                        // add neighbor
                        // send to channel new neighbor command
                // if contains payload

        // socket have data
            // if interface ready
                //read command
                //commands
                    // send
                        // if neighbor not found
                            // send error
                        // send message
                        // update current message state
                    // update beacon payload
                        // update stored beacon payload
}