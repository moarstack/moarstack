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
int freeIfaceDescriptor(void* desc){
	InterfaceDescriptor_T* descriptor = (InterfaceDescriptor_T*)desc;
	unAddressFree(&(descriptor->Address));
	free(descriptor);
}
int interfaceInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int initRes = hashInit(&(layer->Interfaces),intHash, INTERFACES_SIZE, sizeof(int), sizeof(InterfaceDescriptor_T));
	layer->Interfaces.DataFreeFunction = freeIfaceDescriptor;
	return initRes;
}
int interfaceDeinit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
//	//free data here
//	hashIterator_T iterator = {0};
//	int iterRes = hashIterator(&(layer->Interfaces),&iterator);
//	if(FUNC_RESULT_SUCCESS != iterRes)
//		return iterRes;
//	while(!hashIteratorIsLast(&iterator)){
//		int* val = (int*)hashIteratorKey(&iterator);
//		int removeRes = interfaceRemove(layer,*val);
//		hashIteratorNext(&iterator);
//	}
//	layer->InterfacesCount = layer->Interfaces.Count;
	int clearRes = hashClear(&(layer->Interfaces));
	int res = hashFree(&(layer->Interfaces));
	return res;
}
InterfaceDescriptor_T* interfaceFind(ChannelLayer_T *layer, int fd){
	if(NULL == layer)
		return NULL;
	if(fd <= 0)
		return NULL;
	InterfaceDescriptor_T* desc = (InterfaceDescriptor_T*)hashGetPtr(&(layer->Interfaces), &fd);
	return desc;
}

//int interfaceAdd(ChannelLayer_T* layer, InterfaceDescriptor_T* iface){
//	if(NULL == layer)
//		return FUNC_RESULT_FAILED_ARGUMENT;
//	if(NULL == iface)
//		return FUNC_RESULT_FAILED_ARGUMENT;
//	int res = hashAdd(&(layer->Interfaces),&(iface->Socket),&iface);
//	if(FUNC_RESULT_SUCCESS != res)
//		return res;
//	layer->InterfacesCount = layer->Interfaces.Count;
//	return FUNC_RESULT_SUCCESS;
//}
int interfaceAdd(ChannelLayer_T* layer, UnIfaceAddr_T* address, int socket){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	InterfaceDescriptor_T iface = {0};
	iface.Ready = true;
	iface.Socket = socket;
	unAddressClone(address,&(iface.Address));
	int res = hashAdd(&(layer->Interfaces),&socket, &iface);
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
	int removeRes = hashRemove(&(layer->Interfaces),&fd);
	layer->InterfacesCount = layer->Interfaces.Count;
	return removeRes;
}