//
// Created by kryvashek on 13.08.16.
//

#ifndef MOARSTACK_MOARIFACEMOCKITACTIONS_H
#define MOARSTACK_MOARIFACEMOCKITACTIONS_H

#include <moarInterfacePrivate.h>
#include <moarIfaceNeighborsRoutine.h>	// neighborAdd(), neighborUpdate()
#include <moarIfaceCommands.h>			// processCommandIfaceNeighborUpdate(), ...

int actMockitReceived( IfaceState_T * layer );

int actMockitConnection( IfaceState_T * layer );

int actMockitReconnection( IfaceState_T * layer );

#endif //MOARSTACK_MOARIFACEMOCKITACTIONS_H
