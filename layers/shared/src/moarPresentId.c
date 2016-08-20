//
// Created by kryvashek on 18.08.16.
//
#include <moarPresentId.h>
#include <funcResults.h>

// checks whether given message ids are equal; returns true, if are, false otherwise
bool pridAreEqual( PresentId_T * one, PresentId_T * two ) {
	return midAreEqual( ( MessageId_T * )one, ( MessageId_T * )two );
}

// generates new identifier for some packet
PresentId_T pridGenerate( void ) {
	MessageId_T	tempId;

	midGenerate( &tempId, MoarLayer_Presentation );

	return ( PresentId_T )tempId;
}

// converts internal message id into presentation id
PresentId_T pridConvert( MessageId_T internalId ) {
	return ( PresentId_T )internalId;
}