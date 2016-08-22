//
// Created by svalov on 8/22/16.
//

#include "hashFunc.h"

uint32_t hashBytesEx(void* data, size_t size, uint32_t mpc){
		uint32_t hash = 0;
		uint8_t* array = (uint8_t*)data;
		if(NULL != array && 0 != size) {
			uint32_t mp = mpc & 0xFFFFFF;
			uint8_t shift = 32/size;
			if(shift == 0)
				shift = 1;
			for(int i=0;i<size;i++){
				hash <<= shift;
				hash ^= mp*array[i];
			}
		}
		return hash;
}
uint32_t hashInt32(void *data, size_t size){
	return *(uint32_t*)data;
}
uint32_t hashBytes(void* data, size_t size){
	return hashBytesEx(data, size, 0xF423F5);
}
uint32_t hashRoutingAddress(void* address, size_t size){
	return hashBytesEx(address, size, 0xF4E59B);
}
uint32_t hashChannelAddress(void* address, size_t size){
	return hashBytesEx(address, size, 0xF4E613);
}