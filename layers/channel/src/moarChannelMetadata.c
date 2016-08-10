//
// Created by svalov on 7/21/16.
//

#include <moarChannelMetadata.h>
#include <funcResults.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <moarCommons.h>

int writeAddressToMetadata(UnIfaceAddr_T* address, void* plainMetadata, size_t offset){
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == plainMetadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	void* res = memcpy(plainMetadata + offset, address->Value, address->Length);
	return FUNC_RESULT_SUCCESS;
}
//allocate addrSize bytes for address storage
int readAddressFromMetadata(UnIfaceAddr_T* address, UnIfaceAddrLen_T addrSize, void* plainMetadata, size_t offset){
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == plainMetadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	address->Length = addrSize;
	address->Value = malloc(address->Length);
	if(NULL == address->Value)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	void* res = memcpy(address->Value, plainMetadata + offset, addrSize);
	return FUNC_RESULT_SUCCESS;
}

int writeSendMetadata(int fd, ChannelSendMetadata_T* metadata, PayloadSize_T dataSize, void* data){
	if(0 >= fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == metadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 != dataSize && NULL == data)
		return FUNC_RESULT_FAILED_ARGUMENT;

	//fill plain metadata
	size_t metadataSize = CHANNEL_SEND_METADATA_PLAIN_SIZE + metadata->Bridge.Length;
	//
	ChannelSendMetadataPlain_T* sendPlain = malloc(metadataSize);
	if(NULL == sendPlain)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	sendPlain->Id = metadata->Id;
	//copy address to end of metadata
	int addrRes = writeAddressToMetadata(&(metadata->Bridge), sendPlain, CHANNEL_SEND_METADATA_PLAIN_SIZE);
	if(FUNC_RESULT_SUCCESS != addrRes) {
		free(sendPlain);
		return addrRes;
	}
	// fill command
	LayerCommandStruct_T command;
	command.Command = LayerCommandType_Send;
	// metadata + address
	command.MetaSize = metadataSize;
	command.MetaData = (void *)(sendPlain);
	command.DataSize = dataSize;
	command.Data = data;
	// write command
	int res = WriteCommand(fd, &command);
	if(FUNC_RESULT_SUCCESS != res) { //some error
		free(sendPlain);
		return res;
	}
	//free memory
	free(sendPlain);
	//all done
	return FUNC_RESULT_SUCCESS;
}

//allocate addrSize bytes for address storage
//allocate metadata->MessageSize bytes for data storage
int readReceiveMetadata(int fd, LayerCommandStruct_T* command, UnIfaceAddrLen_T addrSize, InterfaceReceiveMetadata_T* metadata){
	if(0 >= fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == metadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == addrSize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(LayerCommandType_Receive != command->Command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// fill metadata from command
	InterfaceReceiveMetadataPlain_T* metadataPlain = (InterfaceReceiveMetadataPlain_T*)command->MetaData;
	if(NULL == metadataPlain)
		return FUNC_RESULT_FAILED_ARGUMENT;
	metadata->Id = metadataPlain->Id;
	//read address from end of metadata
	int addrRes = readAddressFromMetadata(&(metadata->From), addrSize,
										  metadataPlain, INTERFACE_RECEIVE_METADATA_PLAIN_SIZE);
	if(FUNC_RESULT_SUCCESS != addrRes)
		return addrRes;
	//read data
	return FUNC_RESULT_SUCCESS;
}
//allocate metadataPlain->AddressLength bytes for address storage
int readRegisterMetadata(int fd, LayerCommandStruct_T* command, InterfaceRegisterMetadata_T* metadata){
	if(0 >= fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == metadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(LayerCommandType_RegisterInterface != command->Command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// fill metadata from command
	InterfaceRegisterMetadataPlain_T* metadataPlain = (InterfaceRegisterMetadataPlain_T*)command->MetaData;
	if(NULL == metadataPlain)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//read additional metadata here
	//
	//process address
	if(0 == metadataPlain->AddressLength)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//read address from end of metadata
	int addrRes = readAddressFromMetadata(&(metadata->IfaceAddress), metadataPlain->AddressLength,
										  metadataPlain, INTERFACE_REGISTER_METADATA_PLAIN_SIZE);
	if(FUNC_RESULT_SUCCESS != addrRes)
		return addrRes;

	return FUNC_RESULT_SUCCESS;
}
//allocate addrSize bytes for address storage
//allocate metadata->BeaconPayloadSize bytes for data storage
int readNeighborMetadata(int fd, LayerCommandStruct_T* command, UnIfaceAddrLen_T addrSize, InterfaceNeighborMetadata_T* metadata){
	if(0 >= fd)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == metadata)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 == addrSize)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(LayerCommandType_Receive != command->Command)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// fill metadata from command
	InterfaceNeighborMetadataPlain_T* metadataPlain = (InterfaceNeighborMetadataPlain_T*)command->MetaData;
	if(NULL == metadataPlain)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//read address from end of metadata
	int addrRes = readAddressFromMetadata(&(metadata->Address), addrSize,
										  metadataPlain, INTERFACE_NEIGHBOR_METADATA_PLAIN_SIZE);
	if(FUNC_RESULT_SUCCESS != addrRes)
		return addrRes;
	return FUNC_RESULT_SUCCESS;
}