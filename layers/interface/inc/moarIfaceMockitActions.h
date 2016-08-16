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
#define IFACE_MIN_FINISH_POWER	-70 // in dBm
#define IFACE_MAX_START_POWER	16 // in dBm

int actMockitReceived( IfaceState_T * layer );

int actMockitConnection( IfaceState_T * layer );

int actMockitReconnection( IfaceState_T * layer );

#endif //MOARSTACK_MOARIFACEMOCKITACTIONS_H
