//
// Created by svalov on 7/27/16.
//

#include <moarChannelPrivate.h>
#include <funcResults.h>
#include <stdlib.h>
#include <memory.h>
#include <moarChannelInterfaces.h>
#include <moarCommons.h>
#include <moarChannelRouting.h>
#include <moarChannelHello.h>
#include <moarChannelNeighbors.h>

#define COMPARE_EQUAL 		1
#define COMPARE_DIFFERENT 	0
typedef int(*compareFunc_T)(RemoteInterface_T* , RemoteInterface_T*);

int neighborsInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = CreateList(&(layer->Neighbors));
	if(FUNC_RESULT_SUCCESS != res)
		return res;
	int nonresolvedres = CreateList(&(layer->NonResolvedNeighbors));
	if(FUNC_RESULT_SUCCESS != nonresolvedres)
		return nonresolvedres;
	return FUNC_RESULT_SUCCESS;
}

ChannelNeighbor_T* neighborFind(ChannelLayer_T* layer, ChannelAddr_T* address){
	if(NULL == layer)
		return NULL;
	if(NULL == address)
		return NULL;

	LinkedListItem_T* iterator = NextElement(&(layer->Neighbors));
	while(NULL != iterator){
		ChannelNeighbor_T* neighbor = (ChannelNeighbor_T*)iterator->Data;
		int compare = memcmp(address, &(neighbor->RemoteAddress), sizeof(ChannelAddr_T));
		if(0 == compare)
			return neighbor;
		iterator = NextElement(iterator);
	}
	return NULL;
}
RemoteInterface_T* neighborIfaceFind(ChannelNeighbor_T* neighbor){
	if(NULL == neighbor)
		return NULL;

	LinkedListItem_T* iterator = NextElement(&(neighbor->Interfaces));
	while(NULL != iterator){
		RemoteInterface_T* interface = (RemoteInterface_T*)iterator->Data;
		if(interface->BridgeInterface->Ready)
			return interface;
		iterator = NextElement(iterator);
	}
	return NULL;
}
RemoteInterface_T* neighborIfaceFindAddr(ChannelNeighbor_T* neighbor, UnIfaceAddr_T* remoteAddress){
	if(NULL == neighbor)
		return NULL;

	LinkedListItem_T* iterator = NextElement(&(neighbor->Interfaces));
	while(NULL != iterator){
		RemoteInterface_T* interface = (RemoteInterface_T*)iterator->Data;
		if(unAddressCompare(remoteAddress, &(interface->Address)))
			return interface;
		iterator = NextElement(iterator);
	}
	return NULL;
}
int neighborAdd(ChannelLayer_T* layer, ChannelAddr_T* address, UnIfaceAddr_T* remoteAddress, int localSocket){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == remoteAddress)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 >= localSocket)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//if channel address is unknown
	if(NULL == address){
		int res = neighborNonResAdd(layer,remoteAddress,localSocket);
		return res;
	}
	bool isNew = false;
	//find unresolved
	//if found remove
	int removeRes = neighborNonResRemove(layer, remoteAddress);
	// find neighbor
	ChannelNeighbor_T* neighbor = neighborFind(layer, address);
	if(NULL == neighbor){
		// add new
		neighbor = malloc(sizeof(ChannelNeighbor_T));
		if(NULL == neighbor)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		int res = CreateList(&(neighbor->Interfaces));
		if(FUNC_RESULT_SUCCESS != res) {
			free(neighbor);
			return res;
		}
		neighbor->RemoteAddress = *address;
		isNew = true;
	}
	//search interface in neighbor interfaces table
	RemoteInterface_T* remoteInterface = neighborIfaceFindAddr(neighbor, remoteAddress);
	//if not found
	if(NULL == remoteInterface) {
		// add interface here
		remoteInterface = malloc(sizeof(RemoteInterface_T));
		if (NULL == remoteInterface)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;

		remoteInterface->BridgeInterface = interfaceFind(layer, localSocket);
		//copy address
		int cloneRes = unAddressClone(remoteAddress, &(remoteInterface->Address));
		if (FUNC_RESULT_SUCCESS != cloneRes) {
			free(remoteInterface);
			return cloneRes;
		}
		// add to list
		int addRes = AddNext(&(neighbor->Interfaces), remoteInterface);
		if (FUNC_RESULT_SUCCESS != addRes) {
			unAddressFree(&(remoteInterface->Address));
			free(remoteInterface);
			return addRes;
		}

		if (isNew) {
			int addNeighborRes = AddNext(&(layer->Neighbors), neighbor);
			if (FUNC_RESULT_SUCCESS != addNeighborRes) {
				//clean all
				unAddressFree(&(remoteInterface->Address));
				//should be only one item in list
				LinkedListItem_T *item = NextElement(&(neighbor->Interfaces));
				while(NULL != item) {
					free(item->Data); // free(remoteInterface);
					item = DeleteElement(item);
					item = NextElement(item);
				}
				free(neighbor);
				return addNeighborRes;
			}
			// inform routing
			// metadata
			ChannelNeighborMetadata_T neighborMetadata = {0};
			neighborMetadata.Address = *address;
			//commnad
			LayerCommandStruct_T neighborCommand = {0};
			neighborCommand.Command = LayerCommandType_NewNeighbor;
			neighborCommand.MetaData = &neighborMetadata;
			neighborCommand.MetaSize = sizeof(ChannelNeighborMetadata_T);
			int commandRes = WriteCommand(layer->UpSocket, &neighborCommand);
		}
	}
	return FUNC_RESULT_SUCCESS;
}

int cleanNeighbors(ChannelLayer_T* layer, RemoteInterface_T* pattern, compareFunc_T compareFunction){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == pattern)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == compareFunction)
		return FUNC_RESULT_FAILED_ARGUMENT;

	// foreach neighbor
	LinkedListItem_T* iterator = NextElement(&(layer->Neighbors));
	while(NULL != iterator){
		ChannelNeighbor_T* neighbor = (ChannelNeighbor_T*)iterator->Data;
		int interfacesCount = 0;
		LinkedListItem_T* interfaceIterator = NextElement(&(neighbor->Interfaces));
		// foreach interface
		while(NULL != interfaceIterator) {
			RemoteInterface_T* remoteInterface = (RemoteInterface_T*)interfaceIterator->Data;
			int compareRes = compareFunction(remoteInterface, pattern);
			if(COMPARE_EQUAL == compareRes){
				//delete
				unAddressFree(&(remoteInterface->Address));
				free(interfaceIterator->Data);
				interfaceIterator = DeleteElement(interfaceIterator);
			}
			else interfacesCount++;
			interfaceIterator = NextElement(interfaceIterator);
		}
		if(0 == interfacesCount){
			// store address
			ChannelAddr_T address = neighbor->RemoteAddress;
			// remove channel neighbor
			free(iterator->Data);
			iterator = DeleteElement(iterator);
			// inform routing
			//metadata
			ChannelNeighborMetadata_T neighborMetadata = {0};
			neighborMetadata.Address = address;
			//commnad
			LayerCommandStruct_T neighborCommand = {0};
			neighborCommand.Command = LayerCommandType_LostNeighbor;
			neighborCommand.MetaData = &neighborMetadata;
			neighborCommand.MetaSize = sizeof(ChannelNeighborMetadata_T);
			int commandRes = WriteCommand(layer->UpSocket, &neighborCommand);
		}
		iterator = NextElement(iterator);
	}
	return FUNC_RESULT_SUCCESS;
}
int compareByAddress(RemoteInterface_T* value, RemoteInterface_T* reference){
	return (unAddressCompare(&(value->Address),&(reference->Address)))? COMPARE_EQUAL:COMPARE_DIFFERENT;
}
int neighborRemove(ChannelLayer_T* layer, UnIfaceAddr_T remoteAddress){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RemoteInterface_T refInterface;
	refInterface.Address = remoteAddress;
	int cleanRes = cleanNeighbors(layer, &refInterface, compareByAddress);
	return cleanRes;
}
int compareBySocket(RemoteInterface_T* value, RemoteInterface_T* reference){
	return (value->BridgeInterface->Socket == reference->BridgeInterface->Socket)? COMPARE_EQUAL:COMPARE_DIFFERENT;
}
int neighborsRemoveAssociated(ChannelLayer_T* layer, int localSocket){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 >= localSocket)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RemoteInterface_T refInterface;
	refInterface.BridgeInterface = interfaceFind(layer,localSocket);
	int cleanRes = cleanNeighbors(layer, &refInterface, compareBySocket);
	return cleanRes;
}



// non resolved processing
int neighborNonResAdd(ChannelLayer_T* layer, UnIfaceAddr_T* remoteAddress, int localSocket){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == remoteAddress)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 >= localSocket)
		return FUNC_RESULT_FAILED_ARGUMENT;
	NonResolvedNeighbor_T* neighbor = neighborNonResFind(layer, remoteAddress);
	if(NULL == neighbor){
		//add here
		neighbor = malloc(sizeof(NonResolvedNeighbor_T));
		if(NULL == neighbor)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		int addrRes = unAddressClone(remoteAddress, &(neighbor->Address));
		if(FUNC_RESULT_SUCCESS != addrRes)
		{
			free(neighbor);
			return addrRes;
		}
		neighbor->SendAttempts = 0;
		neighbor->NextProcessingTime = timeGetCurrent();
		neighbor->LocalInterfaceSocket = localSocket;
		int addRes = AddNext(&(layer->NonResolvedNeighbors),neighbor);
		if(FUNC_RESULT_SUCCESS != addRes) {
			unAddressFree(&(neighbor->Address));
			free(neighbor);
			return addRes;
		}
	}
	return FUNC_RESULT_SUCCESS;
}
int neighborNonResProcess(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	moarTime_T currentTime = timeGetCurrent();
	LinkedListItem_T* iterator = NextElement(&(layer->NonResolvedNeighbors));
	while(NULL != iterator){
		NonResolvedNeighbor_T* neighbor = (NonResolvedNeighbor_T*)iterator->Data;
		int compRes = timeCompare(currentTime, neighbor->NextProcessingTime);
		if(compRes <= 0){
			neighbor->NextProcessingTime = timeAddInterval(neighbor->NextProcessingTime, PROCESSING_UNRESOLVED_TIMEOUT);
			neighbor->SendAttempts++;
			//find interface
			InterfaceDescriptor_T* iface = interfaceFind(layer, neighbor->LocalInterfaceSocket);
			if(NULL == iface)
				return FUNC_RESULT_FAILED;
			if(iface->Ready) {
				int sendRes = channelHelloSendToNeighbor(layer,&(neighbor->Address),iface);
				//who care?
//				if(FUNC_RESULT_SUCCESS != sendRes)
//					return sendRes;
			}
		}
		iterator = NextElement(iterator);
	}
	return FUNC_RESULT_SUCCESS;
}
int neighborNonResRemove(ChannelLayer_T* layer, UnIfaceAddr_T* address){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	LinkedListItem_T* iterator = NextElement(&(layer->NonResolvedNeighbors));
	while(NULL != iterator){
		NonResolvedNeighbor_T* neighbor = (NonResolvedNeighbor_T*)iterator->Data;
		if(unAddressCompare(address, &(neighbor->Address))){
			//free address
			unAddressFree(&(neighbor->Address));
			//free data
			free(iterator->Data);
			iterator = DeleteElement(iterator);
		}
		iterator = NextElement(iterator);
	}
	return FUNC_RESULT_SUCCESS;
}
NonResolvedNeighbor_T* neighborNonResFind(ChannelLayer_T* layer, UnIfaceAddr_T* address){
	if(NULL == layer)
		return NULL;
	if(NULL == address)
		return NULL;
	LinkedListItem_T* iterator = NextElement(&(layer->NonResolvedNeighbors));
	while(NULL != iterator){
		NonResolvedNeighbor_T* neighbor = (NonResolvedNeighbor_T*)iterator->Data;
		if(unAddressCompare(address, &(neighbor->Address))){
			return neighbor;
		}
		iterator = NextElement(iterator);
	}
	return NULL;
}