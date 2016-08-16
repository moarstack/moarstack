//
// Created by svalov on 7/26/16.
//
#include <moarChannelInterfaces.h>
#include <funcResults.h>
#include <stdlib.h>

int interfaceInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int initRes = CreateList(&(layer->Interfaces));
	return initRes;
}

InterfaceDescriptor_T* interfaceFind(ChannelLayer_T *layer, int fd){
	if(NULL == layer)
		return NULL;
	if(fd <= 0)
		return NULL;
	LinkedListItem_T* iterator = NextElement(&(layer->Interfaces));
	while(NULL != iterator)	{
		InterfaceDescriptor_T* iface = (InterfaceDescriptor_T*)iterator->Data;
		if(fd == iface->Socket)
			return iface;
		iterator = NextElement(iterator);
	}
	return NULL;
}

int interfaceAdd(ChannelLayer_T* layer, InterfaceDescriptor_T* iface){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == iface)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = AddNext(&(layer->Interfaces), iface);
	if(FUNC_RESULT_SUCCESS != res)
		return res;
	layer->InterfacesCount++;
	return FUNC_RESULT_SUCCESS;
}
// remove interface
int interfaceRemove(ChannelLayer_T* layer, int fd){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 >= fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//find
	LinkedListItem_T* iterator = NextElement(&(layer->Interfaces));
	while(NULL != iterator)	{
		InterfaceDescriptor_T* iface = (InterfaceDescriptor_T*)iterator->Data;
		if(fd == iface->Socket){
			int res = unAddressFree(&iface->Address);
			//remove descriptor
			free(iterator->Data);
			//delete element
			LinkedListItem_T* deleteRes = DeleteElement(iterator);
			if(FUNC_RESULT_SUCCESS != res)
				return res;
		}
		iterator = NextElement(iterator);
	}
	return FUNC_RESULT_SUCCESS;
}