//
// Created by svalov on 8/22/16.
//

#ifndef MOARSTACK_HASHFUNC_H
#define MOARSTACK_HASHFUNC_H

#include <stdio.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern uint32_t hashBytesEx(void* data, size_t size, uint32_t mpc);
extern uint32_t hashBytes(void* data, size_t size);
extern uint32_t hashInt(void* data, size_t size);
extern uint32_t hashRoutingAddress(void* address, size_t size);
extern uint32_t hashChannelAddress(void* address, size_t size);

#ifdef __cplusplus
}
#endif

#endif //MOARSTACK_HASHFUNC_H
