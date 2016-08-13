//
// Created by kryvashek on 13.08.16.
//

#ifndef MOARSTACK_MOARIFACECHANNELROUTINE_H
#define MOARSTACK_MOARIFACECHANNELROUTINE_H

#include <moarInterfacePrivate.h>

#define IFACE_PUSH_ATTEMPTS_COUNT	3
#define IFACE_CHANNEL_WAIT_INTERVAL	1 // in seconds

// push to channel layer
int writeUp( IfaceState_T * layer );

// pop from channel layer
int readUp( IfaceState_T * layer );

// connect to channel layer
int connectUp( IfaceState_T * layer );

#endif //MOARSTACK_MOARIFACECHANNELROUTINE_H
