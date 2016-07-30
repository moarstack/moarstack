//
// Created by kryvashek on 30.07.16.
//

#include <moarMessageId.h>

// checks whether given message ids are equal; returns true, if are, false otherwise
extern bool midAreEqual( MessageId_T * one, MessageId_T * two ) {
	return false;
}

// generates new identifier for some packet
extern int midGenerate( MessageId_T * identifier, MoarLayerType_T layer ) {
	return FUNC_RESULT_SUCCESS;
}
