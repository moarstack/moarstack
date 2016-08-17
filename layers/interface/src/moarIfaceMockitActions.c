//
// Created by kryvashek on 13.08.16.
//

#include <moarIfaceMockitActions.h>
#include <moarInterfacePrivate.h>


PowerFloat_T calcMinPower( PowerInt_T startPower, PowerFloat_T finishPower ) {
	PowerFloat_T	neededPower = IFACE_MIN_FINISH_POWER + ( PowerFloat_T )startPower - finishPower;

	return ( IFACE_MAX_START_POWER < neededPower ? IFACE_MAX_START_POWER : neededPower );
}

int actMockitReceivedBeacon( IfaceState_T * layer, IfaceAddr_T * address, PowerFloat_T finishPower ) {
	int				result;
	IfaceNeighbor_T	* sender;
	PowerFloat_T	startPower;

	sender = neighborFind( layer, address );
	startPower = calcMinPower( layer->Memory.BufferFooter.MinSensitivity, finishPower );

	if( NULL != sender ) {
		result = neighborUpdate( layer, sender, startPower );

		if( FUNC_RESULT_SUCCESS == result )
			result = processCommandIfaceNeighborUpdate( layer, address );

	} else {
		result = neighborAdd( layer, address, startPower );

		if( FUNC_RESULT_SUCCESS == result )
			result = processCommandIfaceNeighborNew( layer, address );
	}

	return result;
}

int actMockitRegister( IfaceState_T * layer ) {
	int				result,
					length,
					address;

	memcpy( &address, &( layer->Config.Address ), IFACE_ADDR_SIZE );

	if( 0 == address ) {
		struct timeval	moment;

		gettimeofday( &moment, NULL );
		srand( ( unsigned int )( moment.tv_usec ) );
		address = 1 + rand() % IFACE_ADDRESS_LIMIT;
	}

	snprintf( layer->Memory.Buffer, IFACE_BUFFER_SIZE, "%d%n", address, &length );
	result = writeDown( layer, layer->Memory.Buffer, length );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	memcpy( &( layer->Config.Address ), &address, IFACE_ADDR_SIZE );

	return FUNC_RESULT_SUCCESS;
}

int actMockitRegisterResult( IfaceState_T * layer ) {
	int	result;

	result = readDown( layer, layer->Memory.Buffer, IFACE_BUFFER_SIZE );

	if( 0 >= result )
		return FUNC_RESULT_FAILED_IO;

	layer->Config.IsConnectedToMockit = ( 0 == strncmp( IFACE_REGISTRATION_OK, layer->Memory.Buffer, IFACE_REGISTRATION_OK_SIZE ) );

	if( layer->Config.IsConnectedToMockit ) {
		printf( "Interface registered in MockIT with address %08X\n", *( unsigned int * )&( layer->Config.Address ) );
		fflush( stdout );
		result = FUNC_RESULT_SUCCESS;
	} else
		result = actMockitRegister( layer );

	return result;
}

int actMockitReceived( IfaceState_T * layer ) {
	int				result;
	IfaceNeighbor_T	* sender;
	IfaceAddr_T		address;
	PowerFloat_T	power;

	if( layer->Config.IsConnectedToMockit ) {
		result = receiveAnyData( layer, &power );
		address = layer->Memory.BufferHeader.From;

		if( FUNC_RESULT_SUCCESS == result )
			switch( layer->Memory.BufferHeader.Type ) {
				case IfacePackType_NeedNoResponse:
					break;

				case IfacePackType_NeedResponse :
					sender = neighborFind( layer, &address );
					result = transmitResponse( layer, sender, layer->Memory.BufferHeader.CRC, 0 ); // crc is not implemented yet TODO
					break;

				case IfacePackType_IsResponse :
					// check what message response is for TODO
					result = processCommandIfaceTimeoutFinished( layer, true );
					break;

				case IfacePackType_Beacon :
					result = actMockitReceivedBeacon( layer, &address, power );
					break;

				default :
					result = FUNC_RESULT_FAILED_ARGUMENT;
			}

		if( FUNC_RESULT_SUCCESS == result &&
			IfacePackType_IsResponse != layer->Memory.BufferHeader.Type &&
			0 < layer->Memory.BufferHeader.Size ) // if contains payload
			result = processCommandIfaceReceived( layer );

	} else
		result = actMockitRegisterResult( layer );

	return result;
}

int actMockitConnection( IfaceState_T * layer ) {
	int					result;
	struct epoll_event	* event;

	result = connectDown( layer );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	event = layer->Memory.EpollEvents + 0;
	event->data.fd = layer->Config.MockitSocket;
	event->events = EPOLLIN | EPOLLHUP | EPOLLERR;
	result = epoll_ctl( layer->Config.EpollHandler, EPOLL_CTL_ADD, layer->Config.MockitSocket, event );

	if( FUNC_RESULT_SUCCESS != result )
		return FUNC_RESULT_FAILED;

	result = actMockitRegister( layer );

	return result;
}

int actMockitReconnection( IfaceState_T * layer, bool forced ) {
	int					result;
	struct epoll_event	* event;

	event = layer->Memory.EpollEvents + 0;
	result = epoll_ctl( layer->Config.EpollHandler, EPOLL_CTL_DEL, layer->Config.MockitSocket, event );

	if( FUNC_RESULT_SUCCESS != result )
		return FUNC_RESULT_FAILED;

	if( forced ) {
		shutdown( layer->Config.MockitSocket, SHUT_RDWR );
		close( layer->Config.MockitSocket );
	}

	layer->Config.IsConnectedToMockit = false;
	result = actMockitConnection( layer );

	return result;
}