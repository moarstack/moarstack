//
// Created by kryvashek on 13.08.16.
//

#ifndef MOARSTACK_MOARIFACETRANSMITRECEIVE_H
#define MOARSTACK_MOARIFACETRANSMITRECEIVE_H

#include <moarInterfacePrivate.h>
#include <moarIfacePhysicsRoutine.h>

// receive any kind of data from physical layer
int receiveAnyData( IfaceState_T * layer, PowerFloat_T * finishPower );

// transmit response to the physical layer
int transmitResponse( IfaceState_T * layer, IfaceNeighbor_T * receiver, Crc_T crcInHeader, Crc_T crcFull );

// transmit message to the physical layer
int transmitMessage( IfaceState_T * layer, IfaceNeighbor_T * receiver, bool needResponse );

// transmit beacon to the physical layer
int transmitBeacon( IfaceState_T * layer );

#endif //MOARSTACK_MOARIFACETRANSMITRECEIVE_H
