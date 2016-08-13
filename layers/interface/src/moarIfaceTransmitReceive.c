//
// Created by kryvashek on 13.08.16.
//

#include <moarIfaceTransmitReceive.h>

int getFloat( PowerFloat_T * value, char * buffer, ssize_t * bytesLeft ) {
	char	* end;

	if( NULL == buffer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( NULL == value )
		strtof( buffer, &end );
	else
		*value = strtof( buffer, &end );

	if( NULL != bytesLeft && 0 < *bytesLeft )
		*bytesLeft -= ( end - buffer );

	if( 0 == end - buffer )
		return FUNC_RESULT_FAILED;

	return FUNC_RESULT_SUCCESS;
}

int receiveDataPiece( IfaceState_T * layer, void * destination, ssize_t expectedSize, char ** bufStart, ssize_t * bytesLeft ) {
	ssize_t	bytesDone = 0;

	if( NULL == bufStart || NULL == *bufStart || NULL == bytesLeft )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( 0 < *bytesLeft ) {
		bytesDone = ( *bytesLeft < expectedSize ? *bytesLeft : expectedSize );
		memcpy( destination, *bufStart, bytesDone );
		*bufStart += bytesDone;
		*bytesLeft -= bytesDone;
	}

	if( bytesDone < expectedSize )
		bytesDone += readDown( layer, ( char * )destination + bytesDone, expectedSize - bytesDone );

	if( bytesDone < expectedSize )
		return FUNC_RESULT_FAILED_IO;

	return FUNC_RESULT_SUCCESS;
}

int receiveAnyData( IfaceState_T * layer, PowerFloat_T * finishPower ) {
	ssize_t	bytesLeft,
			result;
	char	* buffer = layer->Memory.Buffer;

	if( NULL == finishPower )
		return FUNC_RESULT_FAILED_ARGUMENT;

	bytesLeft = readDown( layer, buffer, IFACE_BUFFER_SIZE );

	if( -1 == bytesLeft )
		return FUNC_RESULT_FAILED_IO;

	result = getFloat( finishPower, buffer, &bytesLeft ); // we assume that buffer is big enough to contain float

	if( 0 == result )
		return FUNC_RESULT_FAILED_IO; // the strange format is detected if no float value precedes the data, so reading is impossible

	bytesLeft--; // extra byte is for space (aka delimiter) symbol
	buffer++;
	result = receiveDataPiece( layer, &( layer->Memory.BufferHeader ), IFACE_HEADER_SIZE, &buffer, &bytesLeft );

	if( FUNC_RESULT_SUCCESS != result && 0 < layer->Memory.BufferHeader.Size )
		result = receiveDataPiece( layer, layer->Memory.Payload, layer->Memory.BufferHeader.Size, &buffer, &bytesLeft );

	if( FUNC_RESULT_SUCCESS != result && IfacePackType_Beacon == layer->Memory.BufferHeader.Type )
		result = receiveDataPiece( layer, &( layer->Memory.BufferFooter ), IFACE_FOOTER_SIZE, &buffer, &bytesLeft );

	if( FUNC_RESULT_SUCCESS != result )
		return FUNC_RESULT_FAILED_IO;

	return FUNC_RESULT_SUCCESS;
}

int transmitAnyData( IfaceState_T * layer, PowerFloat_T power, void * data, size_t size ) {
	int		currentLength,
			result;

	if( ( NULL == data && 0 < size ) || ( NULL != data && 0 == size ) )
		return FUNC_RESULT_FAILED_ARGUMENT;

	snprintf( layer->Memory.Buffer, IFACE_BUFFER_SIZE, ":%f %n", ( float )power, &currentLength );
	result = writeDown( layer, layer->Memory.Buffer, currentLength );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	snprintf( layer->Memory.Buffer, IFACE_BUFFER_SIZE, "%llu %n", ( long long unsigned )size, &currentLength );
	result = writeDown( layer, layer->Memory.Buffer, currentLength );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = writeDown( layer, data, size );
	return result;
}

int transmitResponse( IfaceState_T * layer, IfaceNeighbor_T * receiver, Crc_T crcInHeader, Crc_T crcFull ) {
	void			* responsePacket,
					* responsePayload;
	int				result;
	size_t			responseSize;
	IfaceHeader_T	* responseHeader;

	if( NULL == receiver )
		return FUNC_RESULT_FAILED_ARGUMENT;

	responseSize = IFACE_HEADER_SIZE + 2 * CRC_SIZE;
	responsePacket = malloc( responseSize );

	if( NULL == responsePacket )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	responseHeader = ( IfaceHeader_T * )responsePacket;
	responsePayload = responsePacket + IFACE_HEADER_SIZE;

	responseHeader->From = layer->Config.Address;
	responseHeader->To = receiver->Address;
	responseHeader->Size = 2 * CRC_SIZE;
	responseHeader->CRC = 0; // that`s not implemented yet TODO
	responseHeader->TxPower = ( PowerInt_T )roundf( receiver->MinPower );
	responseHeader->Type = IfacePackType_IsResponse;

	memcpy( responsePayload, &crcInHeader, CRC_SIZE );
	memcpy( responsePayload + CRC_SIZE, &crcFull, CRC_SIZE );

	result = transmitAnyData( layer, receiver->MinPower, responsePacket, responseSize );
	free( responsePacket );

	return result;
}

int transmitMessage( IfaceState_T * layer, IfaceNeighbor_T * receiver, bool needResponse ) {
	void			* messagePacket,
			* messagePayload;
	int				result;
	size_t			messageSize;
	IfaceHeader_T	* messageHeader;

	if( NULL == receiver )
		return FUNC_RESULT_FAILED_ARGUMENT;

	messageSize = IFACE_HEADER_SIZE + layer->Memory.Command.DataSize;
	messagePacket = malloc( messageSize );

	if( NULL == messagePacket )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	messageHeader = ( IfaceHeader_T * )messagePacket;
	messagePayload = messagePacket + IFACE_HEADER_SIZE;

	messageHeader->From = layer->Config.Address;
	messageHeader->To = receiver->Address;
	messageHeader->Size = layer->Memory.Command.DataSize;
	messageHeader->CRC = 0; // that`s not implemented yet TODO
	messageHeader->TxPower = ( PowerInt_T )roundf( receiver->MinPower );
	messageHeader->Type = ( needResponse ? IfacePackType_NeedResponse : IfacePackType_NeedNoResponse );

	memcpy( messagePayload, layer->Memory.Command.Data, layer->Memory.Command.DataSize );

	result = transmitAnyData( layer, receiver->MinPower, messagePacket, messageSize );
	free( messagePacket );

	return result;
}

int transmitBeacon( IfaceState_T * layer ) {
	void			* beaconPacket,
					* beaconPayload;
	int				result;
	size_t			beaconSize;
	IfaceHeader_T	* beaconHeader;
	IfaceFooter_T	* beaconFooter;

	beaconSize = IFACE_HEADER_SIZE + layer->Config.BeaconPayloadSize + IFACE_FOOTER_SIZE;
	beaconPacket = malloc( beaconSize );

	if( NULL == beaconPacket )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	beaconHeader = ( IfaceHeader_T * )beaconPacket;
	beaconPayload = beaconPacket + IFACE_HEADER_SIZE;
	beaconFooter = ( IfaceFooter_T * )( beaconPayload + layer->Config.BeaconPayloadSize );

	beaconHeader->From = layer->Config.Address;
	memset( &( beaconHeader->To ), 0, IFACE_ADDR_SIZE );
	beaconHeader->Size = layer->Config.BeaconPayloadSize;
	beaconHeader->CRC = 0; // that`s not implemented yet TODO
	beaconHeader->TxPower = ( PowerInt_T )roundf( layer->Config.CurrentBeaconPower );
	beaconHeader->Type = IfacePackType_Beacon;

	memcpy( beaconPayload, layer->Memory.BeaconPayload, layer->Config.BeaconPayloadSize );

	beaconFooter->MinSensitivity = ( PowerInt_T )roundf( layer->Config.CurrentSensitivity );

	result = transmitAnyData( layer, beaconHeader->TxPower, beaconPacket, beaconSize );
	free( beaconPacket );

	return result;
}
