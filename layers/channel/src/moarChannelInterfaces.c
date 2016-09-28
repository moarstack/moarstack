//
// Created by svalov on 7/26/16.
//
#include <moarChannelInterfaces.h>
#include <funcResults.h>
#include <stdlib.h>
#include <moarChannelPrivate.h>
#include <hashTable.h>

hashVal_T intHash(void* val, size_t size) {
	if(NULL == val)
		return 0;
	return *((hashVal_T *)val);
}

int interfaceInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int initRes = hashInit(&(layer->Interfaces),intHash, INTERFACES_SIZE, sizeof(int), sizeof(InterfaceDescriptor_T*));
	return initRes;
}
int interfaceDeinit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//free data here
	hashIterator_T iterator = {0};
	int iterRes = hashIterator(&(layer->Interfaces),&iterator);
	if(FUNC_RESULT_SUCCESS != iterRes)
		return iterRes;
	while(!hashIteratorIsLast(&iterator)){
		int* val = (int*)hashIteratorKey(&iterator);
		int removeRes = interfaceRemove(layer,*val);
		hashIteratorNext(&iterator);
	}
	layer->InterfacesCount = layer->Interfaces.Count;
	int res = hashFree(&(layer->Interfaces));
	return res;
}
InterfaceDescriptor_T* interfaceFind(ChannelLayer_T *layer, int fd){
	if(NULL == layer)
		return NULL;
	if(fd <= 0)
		return NULL;
	InterfaceDescriptor_T* desc;
	int res = hashGet(&(layer->Interfaces), &fd, &desc);
	if(res != FUNC_RESULT_SUCCESS)
		return NULL;
	return desc;
}

int interfaceAdd(ChannelLayer_T* layer, InterfaceDescriptor_T* iface){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == iface)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = hashAdd(&(layer->Interfaces),&(iface->Socket),&iface);
	if(FUNC_RESULT_SUCCESS != res)
		return res;
	layer->InterfacesCount = layer->Interfaces.Count;
	return FUNC_RESULT_SUCCESS;
}
// remove interface
int interfaceRemove(ChannelLayer_T* layer, int fd){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 >= fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//find
	InterfaceDescriptor_T* iface;
	int res = hashGet(&(layer->Interfaces),&fd, &iface);
	if(FUNC_RESULT_SUCCESS != res)
		return FUNC_RESULT_SUCCESS;
	int freeRes = unAddressFree(&(iface->Address));
	int removeRes = hashRemove(&(layer->Interfaces),&fd);
	free(iface);
	layer->InterfacesCount = layer->Interfaces.Count;
	return removeRes;
}