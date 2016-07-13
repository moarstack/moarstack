//
// Created by kryvashek on 09.07.16.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "layerSockets.h"

static int	socketsCountUsual,
			socketsCountIface;
static int	* socketsUsual = NULL,
			* socketsIface = NULL;

int socketDown( MoarLayerType_T layerType, IfacesCount_T ifaceIndex ) {
	switch( layerType ) {
		case MoarLayer_Interface:
			return -1;

		case MoarLayer_Channel:
			return ifaceIndex < socketsCountIface ? socketsIface[ 2 * ifaceIndex ] : -1;

		default:
			return socketsUsual[ 2 * layerType ];
	}
}
// TODO: change both for work with iface-channel sockets
int socketUp( MoarLayerType_T layerType, IfacesCount_T ifaceIndex ) {
	switch( layerType ) {
		case MoarLayer_Service:
			return -1;

		case MoarLayer_Interface:
			return ifaceIndex < socketsCountIface ? socketsIface[ 2 * ifaceIndex + 1 ] : -1;

		default:
			return socketsUsual[ 2 * layerType + 1 ];
	}
}

int socketsPrepare( IfacesCount_T ifacesCount ) {
	int result = 0;

	socketsCountUsual = 2 * MoarLayer_LayersCount - 4; // -1 for beeing between layers, -1 for exceptin iface-channel pair, then * 2
	socketsCountIface = 2 * ifacesCount;

	socketsUsual = ( int * )calloc( socketsCountUsual, sizeof( int ) );
	socketsIface = ( int * )calloc( socketsCountIface, sizeof( int ) );

	for( int i = 0; i < socketsCountUsual; i++ )
		result += socketpair( AF_UNIX, SOCK_DGRAM, 0, socketsUsual + 2 * i );

	for( int i = 0; i < socketsCountIface; i++ )
		result += socketpair( AF_UNIX, SOCK_DGRAM, 0, socketsIface + 2 * i );

	return ( result < 0 ? -1 : 0 );
}