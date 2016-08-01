//
// Created by svalov on 7/21/16.
//

#include <funcResults.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <moarUnIfaceAddr.h>
#include <memory.h>


int ReadUnAddressFromSocket(int fd, UnIfaceAddrLen_T size, UnIfaceAddr_T* address){
	if(0 <= fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//if(NULL != address->Value)
	//	return FUNC_RESULT_FAILED_ARGUMENT;
	address->Length = size;
	//read
	if(0 != size){
		//allocate
		address->Value = malloc(size);
		if(NULL == address->Value)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		//read
		ssize_t addrReaded = read(fd, address->Value, size);
		if(addrReaded != size) {
			free(address->Value);
			address->Value = NULL;
			return FUNC_RESULT_FAILED_IO;
		}
	}
	return FUNC_RESULT_SUCCESS;
}

int WriteUnAddressToSocket(int fd, UnIfaceAddr_T* address){
	if(0 <= fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address->Value)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 != address->Length) {
		ssize_t addrWrited = write(fd, address->Value, address->Length);
		if(addrWrited != address->Length)
			return FUNC_RESULT_FAILED_IO;
	}
	return FUNC_RESULT_SUCCESS;
}

// compare
bool CompareUnAddress(UnIfaceAddr_T* addr1, UnIfaceAddr_T* addr2){
	if(NULL == addr1 || NULL == addr2)
		return false;
	if(addr1->Length != addr2->Length)
		return false;
	for(UnIfaceAddrLen_T index = 0; index< addr1->Length; index++){
		if(addr1->Value[index] != addr2->Value[index])
			return false;
	}
	return true;
}
// make copy
int CloneUnAddress(UnIfaceAddr_T* from, UnIfaceAddr_T* to){
	if( NULL == from)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == from->Value)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == to)
		return FUNC_RESULT_FAILED_ARGUMENT;
	to->Length = from->Length;
	if(0 != from->Length) {
		to->Value = malloc(from->Length);
		if(NULL == to->Value)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		void* res = memcpy(to->Value,from->Value,from->Length);
		if(res != to->Value) {
			free(to->Value);
			to->Value = NULL;
			to->Length = 0;
			return FUNC_RESULT_FAILED;
		}
	}
	return FUNC_RESULT_SUCCESS;
}
// remove
int FreeUnAddress(UnIfaceAddr_T* addr){
	if( NULL == addr)
	return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == addr->Value)
		return FUNC_RESULT_FAILED_ARGUMENT;
	free(addr->Value);
	addr->Value = NULL;
	addr->Length = 0;
	return FUNC_RESULT_SUCCESS;

}