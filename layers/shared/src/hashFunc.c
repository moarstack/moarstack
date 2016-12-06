//
// Created by svalov on 8/22/16.
//

#include <string.h>
#include "hashFunc.h"

hashVal_T hashBytesEx(void* data, size_t size, uint32_t mpc){
	hashVal_T hash = 0;
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
hashVal_T hashInt32(void *data, size_t size){
	return *(hashVal_T*)data;
}
hashVal_T hashBytes(void* data, size_t size){
	return hashBytesEx(data, size, 0xF423F5);
}
hashVal_T hashRoutingAddress(void* address, size_t size){
	return hashBytesEx(address, size, 0xF4E59B);
}
hashVal_T hashChannelAddress(void* address, size_t size){
	return hashBytesEx(address, size, 0xF4E613);
}
hashVal_T hashString(void* data, size_t size){
	size_t len = strlen(data);
	return hashBytesEx(data,len, 0xF4E617);
}