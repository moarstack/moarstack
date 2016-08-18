//
// Created by kryvashek on 30.07.16.
//

#include <string.h> // memcmp()

#include <funcResults.h>
#include <moarMessageId.h>

static uint8_t	valueArray[ MESSAGE_ID_FREE_SIZE ] = { 0 };

// checks whether given message ids are equal; returns true, if are, false otherwise
bool midAreEqual( MessageId_T * one, MessageId_T * two ) {
	if( ( NULL != one && NULL == two ) || ( NULL == one && NULL != two ) )
		return false;
	else if( one == two ) // pointers are equal
		return true;
	else
		return ( 0 == memcmp( one, two, MESSAGE_ID_FULL_SIZE ) );
}

// increments value in specified array
void increment( void ) {
	int i = MESSAGE_ID_FREE_SIZE;

	while( 0 <= --i )
		if( 0 != ++( valueArray[ i ] ) )
			break;
}

// generates new identifier for some packet
int midGenerate( MessageId_T * identifier, MoarLayerType_T layer ) {
	if( NULL == identifier )
		return FUNC_RESULT_FAILED_ARGUMENT;

	increment();
	memcpy( identifier->FreePart, valueArray, MESSAGE_ID_FREE_SIZE );
	identifier->SourceLayer = layer;

	return FUNC_RESULT_SUCCESS;
}
