//
// Created by kryvashek on 09.07.16.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "layerSockets.h"

static int	socketsCount;
static int	* socketValues = NULL;

int socketDown( MoarLayerType_T layerType ) {
	return MoarLayer_Interface == layerType ? -1 : socketValues[ 2 * layerType - 1 ];
}

int socketUp( MoarLayerType_T layerType ) {
	return MoarLayer_Service == layerType ? socketValues[ 2 * layerType ] : -1;
}

int socketFillAddress( struct sockaddr * socketAddress, const char * socketFilePath ) {
	if( NULL == socketFilePath || NULL == socketAddress )
		return -1;

	memset( socketAddress, 0, sizeof( struct sockaddr ) );
	socketAddress->sa_family = AF_UNIX;
	strncpy( socketAddress->sa_data, socketFilePath, SOCKET_FILENAME_SIZE );
	return unlink( socketAddress->sa_data );
}

int socketsPrepare( const char * ifaceSocketFilePath ) {
	struct sockaddr	socketFileAddress;
	int				result = 0;

	socketsCount = 2 * MoarLayer_LayersCount - 2; // -1 for beeing between layers, then * 2
	socketValues = ( int * )calloc( socketsCount, sizeof( int ) );

	socketFillAddress( &socketFileAddress, ifaceSocketFilePath ); // TODO: replace filepath with specified by config file
	socketValues[ 0 ] = socket( AF_UNIX, SOCK_DGRAM, 0 );
	socketValues[ 1 ] = socket( AF_UNIX, SOCK_DGRAM, 0 );
	bind( socketValues[ 1 ], &socketFileAddress, sizeof( struct sockaddr ) );

	for( MoarLayerType_T layerType = MoarLayer_Channel; layerType < MoarLayer_LayersCount; layerType++ )
		result += socketpair( AF_UNIX, SOCK_DGRAM, 0, socketValues + 2 * layerType );

	return ( result < 0 ? -1 : 0 );
}