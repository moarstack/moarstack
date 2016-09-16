//
// Created by svalov on 7/21/16.
//

#ifndef MOARSTACK_MOARUNIFACEADDR_H
#define MOARSTACK_MOARUNIFACEADDR_H

#include <stdint.h>
#include <stdbool.h>
#include <moarInterfaceChannel.h>


// struct for iface address of any length
typedef struct {
	UnIfaceAddrLen_T	Length;
	uint8_t 			* Value;
} UnIfaceAddr_T;

hashVal_T unAddressHash(void* address, size_t size);
// read from socket
int unAddressReadFromSocket(int fd, UnIfaceAddrLen_T size, UnIfaceAddr_T* address);
// write to socket
int unAddressWriteToSocket(int fd, UnIfaceAddr_T* address);
// compare
bool unAddressCompare(UnIfaceAddr_T* addr1, UnIfaceAddr_T* addr2);
// make copy
int unAddressClone(UnIfaceAddr_T* from, UnIfaceAddr_T* to);
// remove
int unAddressFree(UnIfaceAddr_T* addr);

#endif //MOARSTACK_MOARUNIFACEADDR_H
