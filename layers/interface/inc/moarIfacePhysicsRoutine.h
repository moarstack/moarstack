//
// Created by kryvashek on 13.08.16.
//

#ifndef MOARSTACK_MOARIFACEPHYSICSROUTINE_H
#define MOARSTACK_MOARIFACEPHYSICSROUTINE_H

#include <moarInterfacePrivate.h>

#define IFACE_MOCKIT_SOCKET_FILE	"/tmp/mockitSocket.file"
#define IFACE_MOCKIT_WAIT_INTERVAL	1 // in seconds
#define IFACE_SEND_ATTEMPTS_COUNT	3

// send to physics layer
int writeDown( IfaceState_T * layer, void * buffer, int bytes );

// read from physics layer
int readDown( IfaceState_T * layer, void * buffer, int bytes );

// connect to physics layer
int connectDown( IfaceState_T * layer );

#endif //MOARSTACK_MOARIFACEPHYSICSROUTINE_H
