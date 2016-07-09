//
// Created by kryvashek on 09.07.16.
//

#include <sys/types.h>
#include <sys/socket.h>

#include "layerSockets.h"

#define SOCKETPAIRS_COUNT (MoarLayer_LayersCount-1)

int sockets[ 2 * SOCKETPAIRS_COUNT ] = { 0 };

int socketDown( MoarLayerType_T layerType ) {
	return ( MoarLayer_Interface == layerType ? -1 : sockets[ 2 * layerType ] );
}

int socketUp( MoarLayerType_T layerType ) {
	return ( MoarLayer_Service == layerType ? -1 : sockets[ 2 * layerType + 1 ] );
}

int socketsPrepare( void ) {
	int result = 0;

	for( int i = 0; i < SOCKETPAIRS_COUNT; i++ )
		result += socketpair( AF_UNIX, SOCK_DGRAM, 0, sockets + 2 * i );

	return ( result < 0 ? -1 : 0 );
}