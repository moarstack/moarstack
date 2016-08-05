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
	// TODO if contains beacon payload
	// TODO if found neighbor
	// TODO update neighbor
	// TODO else
	// TODO add new neighbor
	// TODO send to routing new neighbor command
	// TODO else
	// TODO add to queue hello message
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
	// TODO if found neighbor
	// TODO remove interface info
	// TODO if no interfaces
	// TODO remove neighbor
	// TODO send to routing lost neighbor command
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
			case LayerCommandType_InterfaceState:
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

	return FUNC_RESULT_SUCCESS;
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

int processQueueEntry(ChannelLayer_T* layer, ChannelMessageEntry_T* entry) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == entry)
		return FUNC_RESULT_FAILED_ARGUMENT;

	return FUNC_RESULT_SUCCESS;
}