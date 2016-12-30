//
// Created by kryvashek on 13.08.16.
//

#ifndef MOARSTACK_MOARIFACEMOCKITACTIONS_H
#define MOARSTACK_MOARIFACEMOCKITACTIONS_H

#include <moarInterfacePrivate.h>
#include <moarIfaceNeighborsRoutine.h>	// neighborAdd(), neighborUpdate()
#include <moarIfaceCommands.h>			// processCommandIfaceNeighborUpdate(), ...

#define IFACE_ADDRESS_LIMIT				10 // sync with the mockit config file
#define IFACE_REGISTRATION_OK			"Registration ok\n" // sync with mockit
#define IFACE_REGISTRATION_OK_SIZE		strlen( IFACE_REGISTRATION_OK )
#define IFACE_POWER_START_MIN			(( PowerFloat_T )0.0) // in dBm
#define IFACE_POWER_START_MAX			(( PowerFloat_T )16.0) // in dBm
#define IFACE_POWER_SAFE_GAP			(( PowerFloat_T )1.0) // in dBm
#define IFACE_ARRAY_MOCKIT_POSITION		0 // position in array preallocated in layer state

int actMockitReceived( IfaceState_T * layer );

int actMockitConnection( IfaceState_T * layer );

int actMockitReconnection( IfaceState_T * layer, bool forced );

#endif //MOARSTACK_MOARIFACEMOCKITACTIONS_H
