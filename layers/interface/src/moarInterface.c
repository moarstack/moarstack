//
// Created by svalov, kryvashek on 05.07.16.
//

#include <moarInterfacePrivate.h>

static IfaceState_T	state = { 0 };

ssize_t writeDown( void * buffer, size_t bytes ) {
	ssize_t result;

	if( ( NULL == buffer && 0 < bytes ) ||
		( NULL != buffer && 0 == bytes ) ||
		FUNC_RESULT_SUCCESS >= state.Config.MockitSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = write( state.Config.MockitSocket, buffer, bytes );

	while( result <= 0 ) {
		sleep( IFACE_WRITE_MOCKIT_INTERVAL );
		result = write( state.Config.MockitSocket, buffer, bytes );
	}

	return result;
}

ssize_t readDown( void * buffer, size_t bytes ) {
	ssize_t	result;

	if( ( NULL == buffer && 0 < bytes ) ||
		( NULL != buffer && 0 == bytes ) ||
		FUNC_RESULT_SUCCESS >= state.Config.MockitSocket )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = read( state.Config.MockitSocket, buffer, bytes );

	while( result <= 0 ) {
		sleep( IFACE_READ_MOCKIT_INTERVAL );
		result = read( state.Config.MockitSocket, buffer, bytes );
	}

	return result;
}
void * MOAR_LAYER_ENTRY_POINT(void* arg){
    // load configuration
    // prepare physical interface
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