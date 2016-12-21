//
// Created by kryvashek on 13.08.16.
//

#ifndef MOARSTACK_MOARIFACECOMMANDS_H
#define MOARSTACK_MOARIFACECOMMANDS_H

#include <moarInterfacePrivate.h>
#include <moarIfaceChannelRoutine.h>	// writeUp(), readUp()
#include <moarIfaceTransmitReceive.h>	// transmitMessage()
#include <moarIfaceNeighborsRoutine.h>	// neighborFind()

#define IFACE_BEACON_INTERVAL			120000 // in milliseconds
#define IFACE_RESPONSE_WAIT_INTERVAL	300 // in milliseconds
#define IFACE_ARRAY_CHANNEL_POSITION	1 // position in array preallocated in layer state

// push to the channel command register
int processCommandIfaceRegister( IfaceState_T * layer );

// push to the channel command message state with value unknown dest
int processCommandIfaceUnknownDest( IfaceState_T * layer );

// push to the channel command message state with value responsed/timeouted
int processCommandIfaceTimeoutFinished( IfaceState_T * layer, bool gotResponse );

// push to the channel command message state with value sent
int processCommandIfaceMessageSent( IfaceState_T * layer );

// push to the channel command new neighbor
int processCommandIfaceNeighborNew( IfaceState_T * layer, IfaceAddr_T * address );

// push to the channel command update neighbor
int processCommandIfaceNeighborUpdate( IfaceState_T * layer, IfaceAddr_T * address );

// push to the channel command received
int processCommandIfaceReceived( IfaceState_T * layer );

// pop from the channel command register result
int processCommandChannelRegisterResult( IfaceState_T * layer, LayerCommandStruct_T * command );

// pop from the channel command send
int processCommandChannelSend( IfaceState_T * layer, LayerCommandStruct_T * command );

// pop from the channel command update beacon payload
int processCommandChannelUpdateBeacon( IfaceState_T * layer, LayerCommandStruct_T * command );

// pop and process any command from the channel
int processCommandChannel( IfaceState_T * layer );

// high-level connection to channel layer
int processChannelConnection( IfaceState_T * layer );

// high-level reconnection to channel layer
int processChannelReconnection( IfaceState_T * layer, bool forced );

#endif //MOARSTACK_MOARIFACECOMMANDS_H
