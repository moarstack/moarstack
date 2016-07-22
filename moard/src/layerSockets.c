//
// Created by kryvashek on 09.07.16.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "layerSockets.h"

static int	socketsCount;
static int	* socketValues = NULL;

int socketDown( MoarLayerType_T layerType ) {
	if( MoarLayer_Interface == layerType || MoarLayer_Service < layerType )
		return -1;
	else
		return socketValues[ 2 * layerType - 2 ];
}

int socketUp( MoarLayerType_T layerType ) {
	if( MoarLayer_Interface == layerType || MoarLayer_Service < layerType )
		return -1;
	else
		return socketValues[ 2 * layerType - 1 ];
}

int socketFillAddress( struct sockaddr_un * socketAddress, const SocketFilepath_T socketFilePath ) {
	if( NULL == socketFilePath || NULL == socketAddress )
		return -1;

	memset( socketAddress, 0, sizeof( struct sockaddr_un ) );
	socketAddress->sun_family = AF_UNIX;
	strncpy( socketAddress->sun_path, socketFilePath, SOCKET_FILEPATH_SIZE );

	if( -1 != access( socketAddress->sun_path, F_OK ) )
		return unlink( socketAddress->sun_path );
	else
		return 0;
}

int socketOpenFile( const SocketFilepath_T socketFilePath ) {
	struct sockaddr_un	socketFileAddress;
	int					socketValue;

	if( -1 == socketFillAddress( &socketFileAddress, socketFilePath ) )
		return -1;

	socketValue = socket( AF_UNIX, SOCK_DGRAM, 0 );

	if( -1 == socketValue )
		return -1;

	return bind( socketValue, ( struct sockaddr * )&socketFileAddress, sizeof( struct sockaddr_un ) );
}

int socketsPrepare( const SocketFilepath_T ifaceSocketFilePath, const SocketFilepath_T serviceSocketFilePath ) {
	int	result = 0;

	socketsCount = 2 + 2 * ( MoarLayer_Service - MoarLayer_Channel );
	socketValues = ( int * )calloc( socketsCount, sizeof( int ) );

	socketValues[ 0 ] = socketOpenFile( ifaceSocketFilePath ); // channel down socket
	socketValues[ socketsCount - 1 ] = socketOpenFile( serviceSocketFilePath ); // service up socket

	result += socketValues[ 0 ] + socketValues[ socketsCount - 1 ];

	for( int i = 1; i < socketsCount - 1; i += 2 )
		result += socketpair( AF_UNIX, SOCK_DGRAM, 0, socketValues + i );

	return ( result < 0 ? -1 : 0 );
}