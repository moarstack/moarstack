//
// Created by svalov on 7/26/16.
//

#include "moarMessageProcessing.h"
#include <funcResults.h>
#include <moarChannelMetadata.h>
#include <stdlib.h>
#include <moarChannelInterfaces.h>
#include <moarInterfaceChannel.h>
#include <moarCommons.h>
#include <moarChannelRouting.h>
#include <moarChannel.h>
#include <memory.h>
#include <moarChannelPrivate.h>
#include <moarChannelNeighbors.h>
#include <moarChannel.h>
#include <moarChannelMessageQueue.h>
#include <moarMessageTable.h>
#include <moarTime.h>


int sendResponseToRouting(ChannelLayer_T* layer, PackStateChannel_T state, RouteSendMetadata_T * metadata){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == metadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(layer->UpSocket <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//fill
	ChannelMessageStateMetadata_T messageStateMetadata;
	messageStateMetadata.State = state;
	messageStateMetadata.Id = metadata->Id;
	// create command
	LayerCommandStruct_T stateCommand = {0};
	stateCommand.MetaData = &messageStateMetadata;
	stateCommand.MetaSize = sizeof(ChannelMessageStateMetadata_T);
	int writeRes = WriteCommand(layer->UpSocket, &stateCommand);
	return writeRes;
}

// register interface
int processRegisterInterface(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	InterfaceRegisterMetadata_T registerMetadata;
	int res = readRegisterMetadata(fd,command, &registerMetadata); //allocate memory for address
	if(FUNC_RESULT_SUCCESS != res)
		return res;

	InterfaceDescriptor_T* ifaceDesc = (InterfaceDescriptor_T*)malloc(sizeof(InterfaceDescriptor_T));
	if(NULL == ifaceDesc)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	ifaceDesc->Socket = fd;
	ifaceDesc->Address = registerMetadata.IfaceAddress;
	ifaceDesc->Neighbors = NULL;
	ifaceDesc->Ready = true;

	int addRes = interfaceAdd(layer, ifaceDesc);
	if(FUNC_RESULT_SUCCESS != addRes){
		unAddressFree(&(ifaceDesc->Address));
		free(ifaceDesc);
	}
	// TODO if supported beacon payload
	// TODO update beacon data
	// TODO update beacon data in all interfaces
	// write registration result to interface
	ChannelRegisterResultMetadata_T resultMetadata;
	//fill metadata
	resultMetadata.Registred = (FUNC_RESULT_SUCCESS == addRes);
	LayerCommandStruct_T resultCommand = {0};
	resultCommand.Command = LayerCommandType_RegisterInterfaceResult;
	resultCommand.MetaData = &resultMetadata;
	resultCommand.MetaSize = CHANNEL_REGISTER_RESULT_METADATA_SIZE;
	int writedRes = WriteCommand(fd, &resultCommand);
	if(FUNC_RESULT_SUCCESS != writedRes)
		return writedRes;
	if(FUNC_RESULT_SUCCESS != addRes)
		return addRes;
	return FUNC_RESULT_SUCCESS;
}
// unregister interface
int processUnregisterInterface(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//close connection
	int res = processCloseConnection(layer,fd);
	if(FUNC_RESULT_SUCCESS != res)
		return res;
	// TODO update beacon data in all interfaces
	return FUNC_RESULT_SUCCESS;
}
// processing received message
int processReceiveMessage(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

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
		if(header->PayloadSize+CHANNEL_LAYER_HEADER_SIZE != command->DataSize){
			free(command->Data);
			command->Data = NULL;
			int addrRes = unAddressFree(&(receiveMetadata.From));
			return FUNC_RESULT_FAILED;
		}
		//add neighbor
		int addRes = neighborAdd(layer, &(header->From), &(receiveMetadata.From), fd);

		if(header->PayloadSize > 0) {
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
				free(command->Data);
				command->Data = NULL;
				int addrRes = unAddressFree(&(receiveMetadata.From));
				return writedRes;
			}
		}
	}
	free(command->Data);
	command->Data = NULL;
	int addrRes = unAddressFree(&(receiveMetadata.From));
	if(FUNC_RESULT_SUCCESS != addrRes)
		return addrRes;
	return FUNC_RESULT_SUCCESS;
}
//process interface state
int processInterfaceState(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
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
				res = sendResponseToRouting(layer, PackStateChannel_Sent, &(entry.Metadata));
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
int processNewNeighbor(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
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
		int addRes = neighborAdd(layer, &(header->From), &(neighborMetadata.Address), fd);
	}
	else{
		// TODO add not resolved processing
	}
	free(command->Data);
	command->Data = NULL;
	int addrRes = unAddressFree(&(neighborMetadata.Address));
	if(FUNC_RESULT_SUCCESS != addrRes)
		return addrRes;
	return FUNC_RESULT_SUCCESS;
}
//process lost neighbor
int processLostNeighbor(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//get
	InterfaceDescriptor_T* interface = interfaceFindBySocket(layer, fd);
	if(NULL == interface)
		return FUNC_RESULT_FAILED_ARGUMENT;
	InterfaceNeighborMetadata_T neighborMetadata = {0};
	int res = readNeighborMetadata(fd, command, interface->Address.Length, &neighborMetadata);
	if(FUNC_RESULT_SUCCESS != res)
		return res;
	int removeRes = neighborRemove(layer, neighborMetadata.Address);
	return FUNC_RESULT_SUCCESS;
}
//process update neighbor
int processUpdateNeighbor(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// TODO ?????
	// TODO PROFIT
	return FUNC_RESULT_SUCCESS;
}
//process message from interface
int processInterfaceData(ChannelLayer_T* layer, int fd, uint32_t event){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;

	// if interface disconnected
	if((event & EPOLL_INTERFACE_DISCONNECT_EVENTS) != 0) {
		// remove from poll list
		int res = processCloseConnection(layer,fd);
		if(FUNC_RESULT_SUCCESS != res)
			return res;
		return FUNC_RESULT_SUCCESS;
	}
		// if command from interface
	else if((event & EPOLL_INTERFACE_EVENTS) !=0) {
		// get command
		LayerCommandStruct_T command = {0};
		int commandRes = ReadCommand(fd, &command);
		if(FUNC_RESULT_SUCCESS != commandRes){
			return commandRes;
		}
		int res;
		switch(command.Command){
			case LayerCommandType_RegisterInterface:
				res = processRegisterInterface(layer, fd, &command);
				break;
			case LayerCommandType_UnregisterInterface:
				res = processUnregisterInterface(layer, fd, &command);
				break;
			case LayerCommandType_MessageState:
				res = processInterfaceState(layer, fd, &command);
				break;
			case LayerCommandType_Receive:
				res = processReceiveMessage(layer, fd, &command);
				break;
			case LayerCommandType_NewNeighbor:
				res = processNewNeighbor(layer, fd, &command);
				break;
			case LayerCommandType_LostNeighbor:
				res = processLostNeighbor(layer, fd, &command);
				break;
			case LayerCommandType_UpdateNeighbor:
				res = processUpdateNeighbor(layer, fd, &command);
				break;
			default:
				res = FUNC_RESULT_FAILED_ARGUMENT;
				break;
		}
		//free metadata
		free(command.MetaData);
		return res;
	}
	return FUNC_RESULT_FAILED;
}

//process send message
int processSendMessage(ChannelLayer_T *layer, int fd, LayerCommandStruct_T *command){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;

	//check metadata
	if(0 == command->MetaSize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command->MetaData)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int res;
	//check for data and data size
	if(0 != command->DataSize && NULL != command->Data) {
		// build message
		PayloadSize_T newSize = command->DataSize + CHANNEL_LAYER_HEADER_SIZE;
		void *newMessage = malloc(newSize);
		if (NULL != newMessage) {
			//fill header
			ChannelLayerHeader_T *header = newMessage;
			header->From = layer->LocalAddress;
			header->PayloadSize = command->DataSize;
			//copy data
			memcpy(newMessage + CHANNEL_LAYER_HEADER_SIZE, command->Data, command->DataSize);
			// add message to queue
			ChannelMessageEntry_T entry = {0};
			entry.DataSize = newSize;
			entry.Data = newMessage;
			entry.TrysLeft = SEND_TRYS;
			entry.ProcessingTime = timeGetCurrent();
			entry.Metadata = *((RouteSendMetadata_T *) command->MetaData);
			res = enqueueMessage(layer, &entry);
		}
		else {
			res = FUNC_RESULT_FAILED_MEM_ALLOCATION;
		}
	}
	else{
		res = sendResponseToRouting(layer, PackStateChannel_NotSent, (RouteSendMetadata_T *) command->MetaData);
	}
	// free old
	free(command->Data);
	command->Data = NULL;
	command->DataSize = 0;
	return res;
}
//process message from routing
int processRoutingData(ChannelLayer_T* layer, int fd, uint32_t event){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(fd <= 0)
		return FUNC_RESULT_FAILED_ARGUMENT;

	if((event & EPOLL_ROUNTING_EVENTS) !=0) {
		// send
		LayerCommandStruct_T command = {0};
		int commandRes = ReadCommand(fd, &command);
		if (FUNC_RESULT_SUCCESS != commandRes) {
			return commandRes;
		}
		int res;
		switch (command.Command) {
			case LayerCommandType_Send:
				res = processSendMessage(layer, fd, &command);
				break;
			default:
				res = FUNC_RESULT_FAILED_ARGUMENT;
				break;
		}
		free(command.MetaData);
		return res;
	}
	return FUNC_RESULT_FAILED;
}

// queue processing
int processQueueEntry(ChannelLayer_T* layer, ChannelMessageEntry_T* entry) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// if not trys left
	if(0 >= entry->TrysLeft){
		int responseRes = sendResponseToRouting(layer, PackStateChannel_NotSent, &(entry->Metadata));
		//free memory here
		free(entry->Data);
		entry->Data = NULL;
		return responseRes;
	}
	// search neighbor
	ChannelNeighbor_T *neighbor = neighborFindByAddress(layer, &(entry->Metadata.Bridge));
	//be sure that find remote can handle null
	if (NULL == neighbor) {
		//no neighbor | interface found
		int responseRes = sendResponseToRouting(layer, PackStateChannel_UnknownDest, &(entry->Metadata));
		//free memory here
		free(entry->Data);
		entry->Data = NULL;
		return responseRes;
	}
	//select interface
	RemoteInterface_T *remoteInterface = neighborFindRemoteInterface(neighbor);
	if(NULL != remoteInterface) {
		// decrement
		entry->TrysLeft--;
		//if interface found
		ChannelSendMetadata_T sendMetadata = {0};
		sendMetadata.Id = entry->Metadata.Id;
		sendMetadata.Bridge = remoteInterface->Address;
		int pushRes = writeSendMetadata(remoteInterface->BridgeInterface->Socket, &sendMetadata, entry->DataSize,
										entry->Data);
		//if pushed down
		if (FUNC_RESULT_SUCCESS == pushRes) {
			remoteInterface->BridgeInterface->Ready = false;
			int res = tableAddEntry(layer, remoteInterface->BridgeInterface, &(entry->Metadata.Id), entry);
			return res;
		}
		else{
			// if can not push reenqueue
			entry->ProcessingTime = timeAddInterval(entry->ProcessingTime, PROCESSING_TIMEOUT);
			int res = enqueueMessage(layer, entry);
			return res;
		}
	}
	else{
		// if not found
		entry->ProcessingTime = timeAddInterval(entry->ProcessingTime, PROCESSING_TIMEOUT);
		int res = enqueueMessage(layer, entry);
		return res;
	}
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
		else{
			done = false;
		}
	}
	return FUNC_RESULT_SUCCESS;
}