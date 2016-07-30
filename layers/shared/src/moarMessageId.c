//
// Created by kryvashek on 30.07.16.
//

#include <string.h> // memcmp()

#include <moarMessageId.h>

// checks whether given message ids are equal; returns true, if are, false otherwise
bool midAreEqual( MessageId_T * one, MessageId_T * two ) {
	if( ( NULL != one && NULL == two ) || ( NULL == one && NULL != two ) )
		return false;
	else if( one == two ) // pointers are equal
		return true;
	else
		return ( 0 == memcmp( one, two, MESSAGE_ID_FULL_SIZE ) );
}

// generates new identifier for some packet
int midGenerate( MessageId_T * identifier, MoarLayerType_T layer ) {
	return FUNC_RESULT_SUCCESS;
}
