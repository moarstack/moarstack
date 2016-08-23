//
// Created by kryvashek on 30.07.16.
//

#include <string.h>			// memcmp()
#include <stdlib.h>			// srand(), rand()

#include <moarMessageId.h>
#include <funcResults.h>
#include <moarTime.h>		// timeGetCurrent()
#include <hashFunc.h>

static uint8_t	nextOrderedValueArray[ MESSAGE_ID_ORDER_SIZE ] = { 0 };
static bool		sranded = false;	// TODO implement normal random and remove that when done

uint32_t midHash(void* id, size_t size){
	return hashBytesEx(id, size, 0xf424d9);
}

// checks whether given message ids are equal; returns true, if are, false otherwise
bool midAreEqual( MessageId_T * one, MessageId_T * two ) {
	if( NULL == one || NULL == two )
		return false;
	else if( one == two ) // pointers are equal
		return true;
	else
		return ( 0 == memcmp( one, two, MESSAGE_ID_FULL_SIZE ) );
}

// increments value in file-statically specified array
void midIncrement( void ) {
	int i = MESSAGE_ID_ORDER_SIZE;

	while( 0 <= --i )
		if( 0 != ++( nextOrderedValueArray[ i ] ) )
			break;
}

// fullfills random part of the message id
int midRandomize( uint8_t * randomPart ) {
	if( NULL == randomPart )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( !sranded ) {
		srand( ( unsigned int )timeGetCurrent() );
		sranded = true; // TODO implement normal random and remove that when done
	}

	for( int i = 0; i < MESSAGE_ID_RAND_SIZE; i++ )
		randomPart[ i ] = ( uint8_t )( rand() & 0xFF );

	return FUNC_RESULT_SUCCESS;
}

// generates new identifier for some packet
int midGenerate( MessageId_T * identifier, MoarLayerType_T layer ) {
	int	result;

	if( NULL == identifier )
		return FUNC_RESULT_FAILED_ARGUMENT;

	identifier->SourceLayer = layer;
	midIncrement();
	memcpy( identifier->PartOrdered, nextOrderedValueArray, MESSAGE_ID_ORDER_SIZE );
	result = midRandomize( identifier->PartRandom );

	return result;
}
