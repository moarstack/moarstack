//
// Created by svalov on 7/29/16.
//

#include <moarChannelPrivate.h>
#include <funcResults.h>
#include <memory.h>
#include "moarMessageTable.h"

const ChannelMessageEntry_T emptyChannelMessageEntry = {0};

int tableFindEntryById(ChannelLayer_T* layer, InterfaceDescriptor_T* iface,  MessageId_T* id, ChannelMessageEntry_T* entry){
	if( NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == iface)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == id)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;
	bool compRes = midAreEqual(id,&(iface->CurrentMessage.Metadata.Id));
	if(compRes)
	{
		*entry = iface->CurrentMessage;
		return FUNC_RESULT_SUCCESS;
	}
	return FUNC_RESULT_FAILED;
}
int tableAddEntry(ChannelLayer_T* layer, InterfaceDescriptor_T* iface,  MessageId_T* id, ChannelMessageEntry_T* entry){
	if( NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == iface)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == id)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int compRes = memcmp(&(iface->CurrentMessage), &emptyChannelMessageEntry, sizeof(ChannelMessageEntry_T));
	if(0 == compRes) {
		iface->CurrentMessage = *entry;
		return FUNC_RESULT_SUCCESS;
	}
	return FUNC_RESULT_FAILED;
}
int tableDeleteEntry(ChannelLayer_T* layer, InterfaceDescriptor_T* iface,  MessageId_T* id){
	if( NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == iface)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if( NULL == id)
		return FUNC_RESULT_FAILED_ARGUMENT;
	bool compRes = midAreEqual(id,&(iface->CurrentMessage.Metadata.Id));
	if(compRes)
	{
		iface->CurrentMessage = emptyChannelMessageEntry;
		return FUNC_RESULT_SUCCESS;
	}
	return FUNC_RESULT_FAILED;
}