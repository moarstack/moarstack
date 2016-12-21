//
// Created by kryvashek on 13.08.16.
//

#include <math.h>						// roundf
#include <moarIfaceTransmitReceive.h>
#include <moarInterfacePrivate.h>
#include <moarTime.h>

int getFloat( PowerFloat_T * value, char * buffer, int * bytesLeft ) {
	char	* end;

	if( NULL == buffer )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( NULL == value )
		strtof( buffer, &end );
	else
		*value = strtof( buffer, &end );

	if( 0 == end - buffer )
		return FUNC_RESULT_FAILED;

	if( NULL != bytesLeft )
		*bytesLeft -= ( end - buffer );

	return FUNC_RESULT_SUCCESS;
}

int receiveDataPiece( IfaceState_T * layer, void * destination, int expectedSize, char ** bufStart, int * bytesLeft ) {
	int	bytesDone = 0,
		newBytes;

	if( NULL == bufStart || NULL == *bufStart || NULL == bytesLeft )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( 0 < *bytesLeft ) {
		bytesDone = ( *bytesLeft < expectedSize ? *bytesLeft : expectedSize );
		memcpy( destination, *bufStart, ( size_t )bytesDone );
		*bufStart += bytesDone;
		*bytesLeft -= bytesDone;
	}

	while( bytesDone < expectedSize ) {
		newBytes = readDown( layer, ( char * )destination + bytesDone, expectedSize - bytesDone );

		if( 0 < newBytes )
			bytesDone += newBytes;
		else if( 0 > newBytes )
			break;
	}

	if( bytesDone < expectedSize )
		return FUNC_RESULT_FAILED_IO;

	return FUNC_RESULT_SUCCESS;
}

int receiveAnyData( IfaceState_T * layer, PowerFloat_T * finishPower ) {
	int		bytesLeft = 0,
			bytesWas,
		 	result = FUNC_RESULT_SUCCESS,
			pos;
	char	* buffer = layer->Memory.Buffer,
			temp;

	memset( buffer, 0, IFACE_BUFFER_SIZE );

	if( NULL == finishPower )
		return FUNC_RESULT_FAILED_ARGUMENT;

	bytesLeft = readDown( layer, buffer, IFACE_BUFFER_SIZE );

	if( 0 >= bytesLeft )
		return FUNC_RESULT_FAILED_IO;

	for( pos = 0; pos < bytesLeft; pos++ )
		if( ' ' != buffer[ pos ] )
			break;

	bytesLeft -= pos;
	buffer += pos;
	bytesWas = bytesLeft;
	result = getFloat( finishPower, buffer, &bytesLeft ); // we assume that buffer is big enough to contain float

	if( FUNC_RESULT_SUCCESS == result ) {
		buffer += bytesWas - bytesLeft;
		bytesLeft--; // extra byte is for space (aka delimiter) symbol
		buffer++;
		result = receiveDataPiece( layer, &( layer->Memory.BufferHeader ), IFACE_HEADER_SIZE, &buffer, &bytesLeft );
		LOG_CHECK_RESULT_MOAR( result, layer->Config.LogHandle, LogLevel_Warning, "error receiving header via mockit", LogLevel_DebugVerbose, "received header via mockit" );

		result = receiveDataPiece( layer, layer->Memory.Payload, ( int )( layer->Memory.BufferHeader.Size ), &buffer, &bytesLeft );
		LOG_CHECK_RESULT_MOAR( result, layer->Config.LogHandle, LogLevel_Warning, "error receiving payload via mockit", LogLevel_DebugVerbose, "received payload via mockit" );

		if( IfacePackType_Beacon == layer->Memory.BufferHeader.Type ) {
			result = receiveDataPiece( layer, &( layer->Memory.BufferFooter ), IFACE_FOOTER_SIZE, &buffer, &bytesLeft );
			LOG_CHECK_RESULT_MOAR( result, layer->Config.LogHandle, LogLevel_Warning, "error receiving footer via mockit", LogLevel_DebugVerbose, "received footer via mockit" );
		}
	} else {
		do
			result = readDown( layer, &temp, 1 );
		while( 1 == result && temp != 0x0A );

		if( 1 == result )
			result = FUNC_RESULT_SUCCESS;
	}

	LogCombMoar( layer->Config.LogHandle, result, LogLevel_Error, "error receiving message via mockit", LogLevel_DebugQuiet, "received message via mockit"  );

	return result;
}

int transmitAnyData( IfaceState_T * layer, PowerFloat_T power, void * data, int size ) {
	int	currentLength,
		result = FUNC_RESULT_SUCCESS;

	if( ( NULL == data && 0 < size ) || ( NULL != data && 0 == size ) )
		result = FUNC_RESULT_FAILED_ARGUMENT;

	if( FUNC_RESULT_SUCCESS == result ) {
		snprintf( layer->Memory.Buffer, IFACE_BUFFER_SIZE, ":%f %n", ( float )power, &currentLength );
		result = writeDown( layer, layer->Memory.Buffer, currentLength );
	}

	if( FUNC_RESULT_SUCCESS == result ) {
		snprintf( layer->Memory.Buffer, IFACE_BUFFER_SIZE, "%d %n", size, &currentLength );
		result = writeDown( layer, layer->Memory.Buffer, currentLength );
	}

	if( FUNC_RESULT_SUCCESS == result )
		result = writeDown( layer, data, size );

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "data transmitting via mockit" );

	return result;
}

int transmitResponse( IfaceState_T * layer, IfaceNeighbor_T * receiver, Crc_T crcInHeader, Crc_T crcFull ) {
	void			* responsePacket,
					* responsePayload;
	int				result = FUNC_RESULT_SUCCESS,
					responseSize;
	IfaceHeader_T	* responseHeader;

	if( NULL == receiver )
		result = FUNC_RESULT_FAILED_ARGUMENT;
	else {
		responseSize = IFACE_HEADER_SIZE + 2 * CRC_SIZE;
		responsePacket = malloc( ( size_t )responseSize );

		if( NULL == responsePacket )
			result = FUNC_RESULT_FAILED_MEM_ALLOCATION;
	}

	if( FUNC_RESULT_SUCCESS == result ) {
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
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "response transmitting" );

	return result;
}

int transmitMessage( IfaceState_T * layer, IfaceNeighbor_T * receiver, bool needResponse, LayerCommandStruct_T * command ) {
	void			* messagePacket,
					* messagePayload;
	int				result = FUNC_RESULT_SUCCESS,
					messageSize;
	IfaceHeader_T	* messageHeader;

	if( NULL == layer || NULL == receiver || NULL == command )
		result = FUNC_RESULT_FAILED_ARGUMENT;
	else {
		messageSize = IFACE_HEADER_SIZE + command->DataSize;
		messagePacket = malloc( ( size_t )messageSize );

		if( NULL == messagePacket )
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	}

	if( FUNC_RESULT_SUCCESS == result ) {
		messageHeader = ( IfaceHeader_T * )messagePacket;
		messagePayload = messagePacket + IFACE_HEADER_SIZE;

		messageHeader->From = layer->Config.Address;
		messageHeader->To = receiver->Address;
		messageHeader->Size = command->DataSize;
		messageHeader->CRC = 0; // that`s not implemented yet TODO
		messageHeader->TxPower = ( PowerInt_T )roundf( receiver->MinPower );
		messageHeader->Type = ( needResponse ? IfacePackType_NeedResponse : IfacePackType_NeedNoResponse );

		memcpy( messagePayload, command->Data, command->DataSize );

		result = transmitAnyData( layer, receiver->MinPower, messagePacket, messageSize );
		free( messagePacket );
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "message transmitting" );

	return result;
}

int transmitBeacon( IfaceState_T * layer ) {
	void			* beaconPacket,
					* beaconPayload;
	int				result = FUNC_RESULT_SUCCESS,
					beaconSize;
	IfaceHeader_T	* beaconHeader;
	IfaceFooter_T	* beaconFooter;

	beaconSize = IFACE_HEADER_SIZE + layer->Config.BeaconPayloadSize + IFACE_FOOTER_SIZE;
	beaconPacket = malloc( ( size_t )beaconSize );

	if( NULL == beaconPacket )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	if( FUNC_RESULT_SUCCESS == result ) {
		beaconHeader = ( IfaceHeader_T * )beaconPacket;
		beaconPayload = beaconPacket + IFACE_HEADER_SIZE;
		beaconFooter = ( IfaceFooter_T * )( beaconPayload + layer->Config.BeaconPayloadSize );

		beaconHeader->From = layer->Config.Address;
		memset( &( beaconHeader->To ), 0, IFACE_ADDR_SIZE );
		beaconHeader->Size = layer->Config.BeaconPayloadSize;
		beaconHeader->CRC = 0xABADBEEF; // that`s not implemented yet TODO
		beaconHeader->TxPower = ( PowerInt_T )roundf( layer->Config.CurrentBeaconPower );
		beaconHeader->Type = IfacePackType_Beacon;

		memcpy( beaconPayload, layer->Memory.BeaconPayload, layer->Config.BeaconPayloadSize );

		beaconFooter->MinSensitivity = ( PowerInt_T )roundf( layer->Config.CurrentSensitivity );

		result = transmitAnyData( layer, beaconHeader->TxPower, beaconPacket, beaconSize );
		free( beaconPacket );
	}

	if( FUNC_RESULT_SUCCESS != result )
		LogErrMoar( layer->Config.LogHandle, LogLevel_Warning, result, "beacon transmitting" );
	else
		layer->Memory.LastBeacon = timeGetCurrent();

	return result;
}
