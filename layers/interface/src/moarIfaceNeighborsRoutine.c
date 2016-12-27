//
// Created by kryvashek on 13.08.16.
//

#include <moarIfaceNeighborsRoutine.h>

IfaceNeighbor_T * neighborFind( IfaceState_T * layer, IfaceAddr_T * address ) {
	for( int i = 0; i < layer->Config.NeighborsCount; i++ )
		if( 0 == memcmp( address, &( layer->Memory.Neighbors[ i ].Address ), IFACE_ADDR_SIZE ) )
			return layer->Memory.Neighbors + i;

	return NULL;
}

int neighborAdd( IfaceState_T * layer, IfaceAddr_T * address, PowerFloat_T minPower ) {
	if( NULL == address )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( IFACE_MAX_NEIGHBOR_COUNT == layer->Config.NeighborsCount )
		return FUNC_RESULT_FAILED;

	layer->Memory.Neighbors[ layer->Config.NeighborsCount ].Address = *address;
	layer->Memory.Neighbors[ layer->Config.NeighborsCount ].MinPower = minPower;
	layer->Memory.Neighbors[ layer->Config.NeighborsCount ].AttemptsLeft = IFACE_DEFAULT_ATTEMPTS_COUNT;
	layer->Memory.Neighbors[ layer->Config.NeighborsCount ].LinkQuality = IFACE_DEFAULT_LINK_QUALITY;
	layer->Config.NeighborsCount++;

	return FUNC_RESULT_SUCCESS;
}

int neighborRemove( IfaceState_T * layer, IfaceNeighbor_T * neighbor ) {
	ptrdiff_t	toPreserve;

	if( NULL == neighbor )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( 0 == layer->Config.NeighborsCount )
		return FUNC_RESULT_FAILED;

	toPreserve = ( layer->Memory.Neighbors + layer->Config.NeighborsCount ) - ( neighbor + 1 );
	memmove( neighbor, neighbor + 1, toPreserve * IFACE_NEIGHBOR_SIZE );
	layer->Config.NeighborsCount--;
	memset( layer->Memory.Neighbors + layer->Config.NeighborsCount, 0, IFACE_NEIGHBOR_SIZE );

	return FUNC_RESULT_SUCCESS;
}

int neighborUpdate( IfaceState_T * layer, IfaceNeighbor_T * neighbor, PowerFloat_T newMinPower ) {
	if( newMinPower < neighbor->MinPower && neighbor->AttemptsLeft < IFACE_DEFAULT_ATTEMPTS_COUNT )
		neighbor->AttemptsLeft++; // neighbor became closer

	neighbor->MinPower = newMinPower;
	return FUNC_RESULT_SUCCESS;
}
