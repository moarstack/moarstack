//
// Created by svalov on 8/22/16.
//

#ifndef MOARSTACK_HASHFUNC_H
#define MOARSTACK_HASHFUNC_H

#include <stdio.h>
#include <stdint.h>
#include <hashTable.h>

__BEGIN_DECLS

extern hashVal_T hashBytesEx(void* data, size_t size, uint32_t mpc);
extern hashVal_T hashBytes(void* data, size_t size);
extern hashVal_T hashInt32(void *data, size_t size);
extern hashVal_T hashRoutingAddress(void* address, size_t size);
extern hashVal_T hashChannelAddress(void* address, size_t size);

__END_DECLS

#endif //MOARSTACK_HASHFUNC_H
