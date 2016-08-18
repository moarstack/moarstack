//
// Created by kryvashek on 18.08.16.
//

#include <string.h> 			// memcmp()

#include <moarPresentId.h>
#include <moarTime.h>			// timeGetCurrent
#include <funcResults.h>

static bool		sranded = false;
static uint8_t	valueArray[ PRESENT_ID_ORDER_SIZE ] = { 0 };

// checks whether given message ids are equal; returns true, if are, false otherwise
bool prIdAreEqual( PresentId_T * one, PresentId_T * two ) {
	if( NULL == one || NULL == two )
		return false;
	else if( one == two ) // pointers are equal
		return true;
	else
		return ( 0 == memcmp( one, two, PRESENT_ID_FULL_SIZE ) );
}

// increments value in specified array
void prIdIncrement( void ) {
	int i = PRESENT_ID_ORDER_SIZE;

	while( 0 <= --i )
		if( 0 != ++( valueArray[ i ] ) )
			break;
}

// prepares randomizing of random part
void prIdSrand( void ) {
	srand( ( unsigned )timeGetCurrent() );
	sranded = true;
}

// returns random value of appropriate type
PresentIdRand_T prIdRandomize( void ) {
	if( !sranded )
		prIdSrand();

	return ( PresentIdRand_T )rand();
}

// generates new identifier for some packet
PresentId_T prIdGenerate( void ) {
	PresentId_T	tempId;

	prIdIncrement();
	memcpy( &( tempId.Order ), valueArray, PRESENT_ID_ORDER_SIZE );
	tempId.Rand = prIdRandomize();

	return tempId;
}

