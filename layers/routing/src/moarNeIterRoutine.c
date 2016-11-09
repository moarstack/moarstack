//
// Created by kryvashek on 09.11.16.
//

#include <moarRoutingPrivate.h>
#include <moarNeIterRoutine.h>
#include <memory.h>

int raslCompareDefault( const void * one, const void * two ) {
	int	comparison = memcmp( one, two, sizeof( RouteAddr_T ) );

	if( 0 > comparison )
		return -1;
	else if( 0 < comparison )
		return 1;
	else
		return 0;
}

int raslInit( RouteAddrSeekList_T * rasl, size_t count, RouteAddrCompare_F compareFunc ) {
	if( NULL == rasl )
		return FUNC_RESULT_FAILED_ARGUMENT;

	rasl->List = malloc( count * sizeof( RouteAddr_T ) );

	if( 0 < count && NULL == rasl->List )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	memset( rasl->List, 0, count * sizeof( RouteAddr_T ) );
	rasl->Count = count;
	rasl->CompareFunc = ( NULL == compareFunc ? raslCompareDefault : compareFunc );
	return FUNC_RESULT_SUCCESS;
}

int raslSet( RouteAddrSeekList_T * rasl, size_t start, RouteAddr_T * source, size_t count ) {
	if( NULL == rasl || NULL == source || rasl->Count <= start + count )
		return FUNC_RESULT_FAILED_ARGUMENT;

	memcpy( rasl->List + start, source, count * sizeof( RouteAddr_T ) );
	return FUNC_RESULT_SUCCESS;
}

int raslSort( RouteAddrSeekList_T * rasl ) {
	if( NULL == rasl || NULL == rasl->CompareFunc )
		return FUNC_RESULT_FAILED_ARGUMENT;

	qsort( rasl->List, rasl->Count, sizeof( RouteAddr_T ), rasl->CompareFunc );
}

RouteAddr_T * raslSeek( RouteAddrSeekList_T * rasl, RouteAddr_T * needle ) {
	return ( RouteAddr_T * )bsearch( needle, rasl->List, rasl->Count, sizeof( RouteAddr_T ), rasl->CompareFunc );
}

int raslDeinit( RouteAddrSeekList_T * rasl ) {
	if( NULL == rasl )
		return FUNC_RESULT_FAILED_ARGUMENT;

	free( rasl->List );
	rasl->List = NULL;
	return FUNC_RESULT_SUCCESS;
}

int neIterShift( hashIterator_T * iterator, size_t shift ) {
	size_t	index;
	int 	result = FUNC_RESULT_SUCCESS;

	if( NULL == iterator )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( index = 0; FUNC_RESULT_SUCCESS == result && index < shift; index++ )
		if( hashIteratorIsLast( iterator ) )
			result = FUNC_RESULT_FAILED;
		else
			result = hashIteratorNext( iterator );

	return result;
}

int neIterShiftWhileNull( hashIterator_T * iterator, size_t limit ) {
	size_t	index;
	int 	result = FUNC_RESULT_SUCCESS;

	if( NULL == iterator )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( index = 0; FUNC_RESULT_SUCCESS == result && NULL == hashIteratorData( iterator ) && index < limit; index++ )
		if( hashIteratorIsLast( iterator ) )
			result = FUNC_RESULT_FAILED;
		else
			result = hashIteratorNext( iterator );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	return ( NULL == hashIteratorData( iterator ) ? FUNC_RESULT_FAILED : FUNC_RESULT_SUCCESS );
}

bool neIterCheckNullOrUsed( hashIterator_T * iterator, RouteAddrSeekList_T * rasl ) {
	RoutingNeighborInfo_T	* neInfo = hashIteratorData( iterator );

	if( NULL == neInfo )
		return true;
	else
		return NULL != raslSeek( rasl, &( neInfo->Address ) );
}

int neIterShiftWhileUsed( hashIterator_T * iterator, size_t limit, RouteAddrSeekList_T * rasl ) {
	size_t	index;
	int 	result = FUNC_RESULT_SUCCESS;

	if( NULL == iterator )
		return FUNC_RESULT_FAILED_ARGUMENT;

	for( index = 0; FUNC_RESULT_SUCCESS == result && neIterCheckNullOrUsed( iterator, rasl ) && index < limit; index++ )
		if( hashIteratorIsLast( iterator ) )
			result = FUNC_RESULT_FAILED;
		else
			result = hashIteratorNext( iterator );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	return ( neIterCheckNullOrUsed( iterator, rasl ) ? FUNC_RESULT_FAILED : FUNC_RESULT_SUCCESS );
}

int neIterFindRandNotNUll( RoutingNeighborsStorage_T * storage, RouteAddr_T * address ) {
	hashIterator_T	iterator = { 0 };
	size_t			index;
	int 			result;

	if( NULL == storage || NULL == address )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = storageIterator( storage, &iterator );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	index = ( size_t )( rand() % storage->Count );
	result = neIterShift( &iterator, index );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = neIterShiftWhileNull( &iterator, storage->Count - index );

	if( FUNC_RESULT_SUCCESS != result ) {
		result = storageIterator( storage, &iterator );

		if( FUNC_RESULT_SUCCESS != result )
			return result;

		result = neIterShiftWhileNull( &iterator, index );
	}

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	*address = ( ( RoutingNeighborInfo_T * )hashIteratorData( &iterator ) )->Address;

	return FUNC_RESULT_SUCCESS;
}

int neIterFindRandNotNUllOrUsed( RoutingNeighborsStorage_T * storage, RouteAddrSeekList_T * rasl, RouteAddr_T * address ) {
	hashIterator_T	iterator = { 0 };
	size_t			index;
	int 			result;

	if( NULL == storage || NULL == address )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = storageIterator( storage, &iterator );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	index = ( size_t )( rand() % storage->Count );
	result = neIterShift( &iterator, index );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	result = neIterShiftWhileUsed( &iterator, storage->Count - index, rasl );

	if( FUNC_RESULT_SUCCESS != result ) {
		result = storageIterator( storage, &iterator );

		if( FUNC_RESULT_SUCCESS != result )
			return result;

		result = neIterShiftWhileUsed( &iterator, index, rasl );
	}

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	*address = ( ( RoutingNeighborInfo_T * )hashIteratorData( &iterator ) )->Address;

	return FUNC_RESULT_SUCCESS;
}