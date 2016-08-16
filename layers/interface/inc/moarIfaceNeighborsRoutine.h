//
// Created by kryvashek on 13.08.16.
//

#ifndef MOARSTACK_MOARIFACENEIGHBORSROUTINE_H
#define MOARSTACK_MOARIFACENEIGHBORSROUTINE_H

#include <stddef.h>	// ptrdiff_t
#include <stdlib.h>	// memmove()

#include <moarInterfacePrivate.h>

#define IFACE_NEIGHBOR_SIZE				sizeof( IfaceNeighbor_T )
#define IFACE_DEFAULT_LINK_QUALITY		0.5
#define IFACE_DEFAULT_ATTEMPTS_COUNT	3

// find neighbor by its address; returns NULL if such neighbor isn`t presented
IfaceNeighbor_T * neighborFind( IfaceState_T * layer, IfaceAddr_T * address );

// add neighbor with specified address and minimal power to make him hear
int neighborAdd( IfaceState_T * layer, IfaceAddr_T * address, PowerFloat_T minPower );

// remove specified neighbor
int neighborRemove( IfaceState_T * layer, IfaceNeighbor_T * neighbor );

// update specified neighbor
int neighborUpdate( IfaceState_T * layer, IfaceNeighbor_T * neighbor, PowerFloat_T newMinPower );

#endif //MOARSTACK_MOARIFACENEIGHBORSROUTINE_H
