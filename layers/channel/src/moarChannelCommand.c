//
// Created by svalov on 7/26/16.
//

#include "moarChannelCommand.h"
#include <funcResults.h>
#include <moarChannelMetadata.h>
#include <stdlib.h>
#include <moarChannelInterfaces.h>
#include <memory.h>
#include <moarChannelNeighbors.h>
#include <moarChannelMessageQueue.h>
#include <moarChannelMessageTable.h>
#include <moarChannelHello.h>


int sendResponseToRouting(ChannelLayer_T* layer, PackStateChannel_T state, RouteSendMetadata_T * metadata, SendTrys_T sendTrys){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == metadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(layer->UpSocket <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//fill
	ChannelMessageStateMetadata_T messageStateMetadata;
	messageStateMetadata.State = state;
	messageStateMetadata.SentTrys = sendTrys;
	messageStateMetadata.Id = metadata->Id;
	// create command
	LayerCommandStruct_T stateCommand = {0};
	stateCommand.MetaData = &messageStateMetadata;
	stateCommand.MetaSize = sizeof(ChannelMessageStateMetadata_T);
	int writeRes = WriteCommand(layer->UpSocket, &stateCommand);
	return writeRes;
}

// register interface
int processRegisterInterface(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ChannelLayer_T* layer = (ChannelLayer_T*)layerRef;
	InterfaceRegisterMetadata_T registerMetadata;

	int res = readRegisterMetadata(fd,command, &registerMetadata); //allocate memory for address
	if(FUNC_RESULT_SUCCESS != res)
		return res;

	InterfaceDescriptor_T* ifaceDesc = (InterfaceDescriptor_T*)malloc(sizeof(InterfaceDescriptor_T));
	if(NULL == ifaceDesc) {
		unAddressFree(&(registerMetadata.IfaceAddress));
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	}
	ifaceDesc->Socket = fd;
	ifaceDesc->Address = registerMetadata.IfaceAddress;
	ifaceDesc->Neighbors = NULL;
	ifaceDesc->Ready = true;

	int addRes = interfaceAdd(layer, ifaceDesc);

	// write registration result to interface
	ChannelRegisterResultMetadata_T resultMetadata;
	//fill metadata
	resultMetadata.Registred = (FUNC_RESULT_SUCCESS == addRes);
	LayerCommandStruct_T resultCommand = {0};
	resultCommand.Command = LayerCommandType_RegisterInterfaceResult;
	resultCommand.MetaData = &resultMetadata;
	resultCommand.MetaSize = CHANNEL_REGISTER_RESULT_METADATA_SIZE;
	int writedRes = WriteCommand(fd, &resultCommand);

	if(FUNC_RESULT_SUCCESS != addRes) {
		// if can not add interface
		unAddressFree(&(ifaceDesc->Address));
		free(ifaceDesc);
		return addRes;
	}
	//here interface added  to table
	if(FUNC_RESULT_SUCCESS != writedRes)
		return writedRes;
	// update hello packet
	int helloRes = channelHelloFill(layer);
	if(FUNC_RESULT_SUCCESS != helloRes)
		return helloRes;
	// spread hello to interfaces
	int ifaceRes = channelHelloUpdateInterface(layer);
	if(FUNC_RESULT_SUCCESS != ifaceRes)
		return helloRes;

	return FUNC_RESULT_SUCCESS;
}
// unregister interface
int processUnregisterInterface(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	ChannelLayer_T* layer = (ChannelLayer_T*)layerRef;
	//close connection
	int res = processCloseConnection(layer,fd);
	if(FUNC_RESULT_SUCCESS != res)
		return res;

	return FUNC_RESULT_SUCCESS;
}
// processing received message
int processReceiveMessage(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ChannelLayer_T* layer = (ChannelLayer_T*)layerRef;

	InterfaceDescriptor_T* interface = interfaceFindBySocket(layer, fd);
	if(NULL == interface)
		return FUNC_RESULT_FAILED_ARGUMENT;

	InterfaceReceiveMetadata_T receiveMetadata = {0};
	int res = readReceiveMetadata(fd, command, interface->Address.Length, &receiveMetadata);
	if(FUNC_RESULT_SUCCESS != res)
		return res;

	// if contains payload
	if(command->DataSize > 0) {
		// extract header
		ChannelLayerHeader_T* header = (ChannelLayerHeader_T*)(command->Data);
		// check payload size
		if(header->PayloadSize + CHANNEL_LAYER_HEADER_SIZE != command->DataSize) {
			unAddressFree(&(receiveMetadata.From));
			return FUNC_RESULT_FAILED;
		}
		//add neighbor
		neighborAdd(layer, &(header->From), &(receiveMetadata.From), fd);
		//check packet type
		if(header->Hello){
			channelHelloProcess(layer, command->DataSize, command->Data);
		}
		// normal data
		else if(header->PayloadSize > 0) {
			//create command
			LayerCommandStruct_T channelReceiveCommand = {0};
			channelReceiveCommand.Command = LayerCommandType_Receive;
			channelReceiveCommand.Data = (command->Data + CHANNEL_LAYER_HEADER_SIZE);
			channelReceiveCommand.DataSize = header->PayloadSize;
			//fill metadata
			ChannelReceiveMetadata_T channelReceiveMetadata = {0};
			channelReceiveMetadata.From = header->From;
			//add to command
			channelReceiveCommand.MetaData = &channelReceiveMetadata;
			channelReceiveCommand.MetaSize = sizeof(ChannelReceiveMetadata_T);
			//write
			int writedRes = WriteCommand(layer->UpSocket, &channelReceiveCommand);
			if (FUNC_RESULT_SUCCESS != writedRes) {
				unAddressFree(&(receiveMetadata.From));
				return writedRes;
			}
		}
	}
	unAddressFree(&(receiveMetadata.From));
	return FUNC_RESULT_SUCCESS;
}
//process interface state
int processInterfaceState(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ChannelLayer_T* layer = (ChannelLayer_T*)layerRef;

	//get interface
	InterfaceDescriptor_T* ifaceDesc = interfaceFindBySocket(layer, fd);
	if(NULL == ifaceDesc)
		return FUNC_RESULT_FAILED_ARGUMENT;

	//set free
	ifaceDesc->Ready = true;
	//get metadata
	IfacePackStateMetadata_T* metadata = (IfacePackStateMetadata_T*)command->MetaData;

	//find entry in table
	ChannelMessageEntry_T entry;
	int findRes = tableFindEntryById(layer, ifaceDesc, &(metadata->Id), &entry);
	if(FUNC_RESULT_SUCCESS == findRes) {
		// drop entry from table
		tableDeleteEntry(layer, ifaceDesc, &(metadata->Id));
		//process
		int res;
		switch (metadata->State) {
			case IfacePackState_UnknownDest:
			case IfacePackState_Timeouted:
				res = enqueueMessage(layer, &entry);
				break;
			case IfacePackState_Sent:
			case IfacePackState_Responsed:
				//notify sent
				res = sendResponseToRouting(layer, PackStateChannel_Sent, &(entry.Metadata), entry.SendTrys);
				// drop message
				free(entry.Data);
				entry.Data = NULL;
				// drop entry
				break;
			default:
				res = FUNC_RESULT_FAILED_ARGUMENT;
				break;
		}
		return res;
	}
	return FUNC_RESULT_SUCCESS;
}
//process new neighbor
int processNewNeighbor(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	ChannelLayer_T* layer = (ChannelLayer_T*)layerRef;
	//get
	InterfaceDescriptor_T* interface = interfaceFindBySocket(layer, fd);
	if(NULL == interface)
		return FUNC_RESULT_FAILED_ARGUMENT;

	InterfaceNeighborMetadata_T neighborMetadata = {0};
	int res = readNeighborMetadata(fd, command, interface->Address.Length, &neighborMetadata);
	if(FUNC_RESULT_SUCCESS != res)
		return res;

	// if contains payload
	if(command->DataSize > 0) {
		// extract header
		ChannelLayerHeader_T* header = (ChannelLayerHeader_T*)(command->Data);
		//add neighbor
		neighborAdd(layer, &(header->From), &(neighborMetadata.Address), fd);
		//check packet type
		if(header->Hello)
			channelHelloProcess(layer, command->DataSize, command->Data);
	}
	else
		neighborAdd(layer, NULL, &(neighborMetadata.Address), fd);

	unAddressFree(&(neighborMetadata.Address));
	return FUNC_RESULT_SUCCESS;
}
//process lost neighbor
int processLostNeighbor(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	ChannelLayer_T* layer = (ChannelLayer_T*)layerRef;
	//get
	InterfaceDescriptor_T* interface = interfaceFindBySocket(layer, fd);
	if(NULL == interface)
		return FUNC_RESULT_FAILED_ARGUMENT;
	InterfaceNeighborMetadata_T neighborMetadata = {0};
	int res = readNeighborMetadata(fd, command, interface->Address.Length, &neighborMetadata);
	if(FUNC_RESULT_SUCCESS != res)
		return res;
	int removeRes = neighborRemove(layer, neighborMetadata.Address);
	unAddressFree(&(neighborMetadata.Address));
	return removeRes;
}
//process update neighbor
int processUpdateNeighbor(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	ChannelLayer_T* layer = (ChannelLayer_T*)layerRef;
	// TODO ?????
	// TODO PROFIT
	return FUNC_RESULT_SUCCESS;
}

//process send message
int processSendMessage(void* layerRef, int fd, LayerCommandStruct_T *command){
	if(NULL == layerRef)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	ChannelLayer_T* layer = (ChannelLayer_T*)layerRef;
	//check metadata
	if(0 == command->MetaSize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command->MetaData)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res = FUNC_RESULT_FAILED;
	//check for data and data size
	if(0 != command->DataSize && NULL != command->Data) {
		// build message
		PayloadSize_T newSize = command->DataSize + CHANNEL_LAYER_HEADER_SIZE;
		void *newMessage = malloc(newSize);
		if (NULL != newMessage) {
			//fill header
			ChannelLayerHeader_T *header = newMessage;
			header->From = layer->LocalAddress;
			header->PayloadSize = (PayloadSize_T)command->DataSize; // may be overflow
			//assert for correct size
			if(header->PayloadSize != command->DataSize){
				free(newMessage);
				return FUNC_RESULT_FAILED_ARGUMENT;
			}
			//copy data
			memcpy(newMessage + CHANNEL_LAYER_HEADER_SIZE, command->Data, command->DataSize);
			// add message to queue
			ChannelMessageEntry_T entry = {0};
			entry.DataSize = newSize;
			entry.Data = newMessage;
			entry.SendTrys = 0;
			entry.ProcessingTime = timeGetCurrent();
			entry.Metadata = *((RouteSendMetadata_T *) command->MetaData);
			res = enqueueMessage(layer, &entry);
			if(FUNC_RESULT_SUCCESS != res)
				free(newMessage);
		}
		else
			res = FUNC_RESULT_FAILED_MEM_ALLOCATION;
	}
	else
		res = sendResponseToRouting(layer, PackStateChannel_NotSent, (RouteSendMetadata_T *) command->MetaData, 0);
	return res;
}

// queue processing
int processQueueEntry(ChannelLayer_T* layer, ChannelMessageEntry_T* entry) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;

	ChannelNeighbor_T *neighbor = NULL;
	PackStateChannel_T state = PackStateChannel_None;

	// if not trys left
	if(entry->SendTrys <= SEND_TRYS){
		// search neighbor
		neighbor = neighborFindByAddress(layer, &(entry->Metadata.Bridge));
		//be sure that find remote can handle null
		if (NULL == neighbor) //no neighbor | interface found
			state = PackStateChannel_UnknownDest;
	} else
		state = PackStateChannel_NotSent;

	//if state changed due to errors
	if(PackStateChannel_None != state){
		int responseRes = sendResponseToRouting(layer, state, &(entry->Metadata), entry->SendTrys);
		//free memory here
		free(entry->Data);
		entry->Data = NULL;
		return responseRes;
	}
	// decrement
	entry->SendTrys++;

	//select interface
	RemoteInterface_T *remoteInterface = neighborFindRemoteInterface(neighbor);
	if(NULL != remoteInterface) {
		//if interface found
		ChannelSendMetadata_T sendMetadata = {0};
		sendMetadata.Id = entry->Metadata.Id;
		sendMetadata.Bridge = remoteInterface->Address;
		sendMetadata.NeedResponse = DATA_NEED_RESPONSE;
		int pushRes = writeSendMetadata(remoteInterface->BridgeInterface->Socket, &sendMetadata, entry->DataSize,
										entry->Data);
		//if pushed down
		if (FUNC_RESULT_SUCCESS == pushRes) {
			remoteInterface->BridgeInterface->Ready = false;
			int res = tableAddEntry(layer, remoteInterface->BridgeInterface, &(entry->Metadata.Id), entry);
			return res;
		}
	}
	// if not found | not pushed
	entry->ProcessingTime = timeAddInterval(entry->ProcessingTime, PROCESSING_TIMEOUT);
	int res = enqueueMessage(layer, entry);
	return res;
}
// process channel message queue
int processQueue(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//get from queue top
	ChannelMessageEntry_T * entry;
	bool done = true;
	moarTime_T currentTime = timeGetCurrent();
	while(done) {
		int peekRes = peekMessage(layer, &entry);
		//if no data in queue
		if (FUNC_RESULT_SUCCESS != peekRes)
			return FUNC_RESULT_SUCCESS;
		int compare = timeCompare(currentTime, entry->ProcessingTime);
		if(compare > 0) {
			//process, who cares about result
			int processRes = processQueueEntry(layer, entry);
			if (FUNC_RESULT_SUCCESS != processRes)
				done = false;
			//remove from queue
			dequeueMessage(layer, NULL);
		}
		else
			done = false;
	}
	return FUNC_RESULT_SUCCESS;
}