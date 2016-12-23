//
// Created by kryvashek on 09.07.16.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <layerSockets.h>
#include <funcResults.h>

static int	socketsCount;
static int	* socketValues = NULL;

int SocketDown( MoarLayerType_T layerType ) {
	if( MoarLayer_Interface == layerType || MoarLayer_Service < layerType )
		return FUNC_RESULT_FAILED;
	else
		return socketValues[ 2 * layerType - 2 ];
}

int SocketUp( MoarLayerType_T layerType ) {
	if( MoarLayer_Interface == layerType || MoarLayer_Service < layerType )
		return FUNC_RESULT_FAILED;
	else
		return socketValues[ 2 * layerType - 1 ];
}

int SocketsPrepare( const SocketFilepath_T ifaceSocketFilePath, const SocketFilepath_T serviceSocketFilePath ) {
	int	result = 0;

	socketsCount = 2 + 2 * ( MoarLayer_Service - MoarLayer_Channel );
	socketValues = ( int * )calloc( socketsCount, sizeof( int ) );

	result = SocketOpenFile( ifaceSocketFilePath, true, socketValues ); // channel down socket
	CHECK_RESULT( result );

	result = SocketOpenFile( serviceSocketFilePath, true, socketValues + socketsCount - 1 ); // service up socket
	CHECK_RESULT( result );

	result = 0;

	for( int i = 1; i < socketsCount - 1; i += 2 )
		result += socketpair( AF_UNIX, SOCK_DGRAM, 0, socketValues + i );

	return ( result == 0 ? FUNC_RESULT_SUCCESS : FUNC_RESULT_FAILED );
}