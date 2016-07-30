//
// Created by kryvashek on 30.07.16.
//

#include <string.h> // memcmp()

#include <moarMessageId.h>

static uint8_t	value[ MESSAGE_ID_FULL_SIZE ] = { 0 };

// checks whether given message ids are equal; returns true, if are, false otherwise
bool midAreEqual( MessageId_T * one, MessageId_T * two ) {
	if( ( NULL != one && NULL == two ) || ( NULL == one && NULL != two ) )
		return false;
	else if( one == two ) // pointers are equal
		return true;
	else
		return ( 0 == memcmp( one, two, MESSAGE_ID_FULL_SIZE ) );
}

int increment( uint8_t * valueArray ) {
	if( NULL == valueArray )
		return FUNC_RESULT_FAILED_ARGUMENT;

	valueArray[ MESSAGE_ID_FULL_SIZE - 1 ]++;

	return FUNC_RESULT_SUCCESS;
}

// generates new identifier for some packet
int midGenerate( MessageId_T * identifier, MoarLayerType_T layer ) {
	int result;

	if( NULL == identifier )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = increment( value );

	if( FUNC_RESULT_SUCCESS != result )
		return FUNC_RESULT_FAILED;

	memcmp( identifier, value, MESSAGE_ID_FULL_SIZE );

	if( MoarLayer_LayersCount > layer )
		identifier->SourceLayer = layer;

	return FUNC_RESULT_SUCCESS;
}
