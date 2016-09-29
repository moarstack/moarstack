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
#include <hashTable.h>

int notifyRouting(ChannelLayer_T* layer, LayerCommandType_T type, ChannelAddr_T* address){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(LayerCommandType_LostNeighbor != type && LayerCommandType_NewNeighbor != type)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// metadata
	ChannelNeighborMetadata_T neighborMetadata = {0};
	neighborMetadata.Address = *address;
	//commnad
	LayerCommandStruct_T neighborCommand = {0};
	neighborCommand.Command = type;
	neighborCommand.MetaData = &neighborMetadata;
	neighborCommand.MetaSize = sizeof(ChannelNeighborMetadata_T);
	int commandRes = WriteCommand(layer->UpSocket, &neighborCommand);
	return commandRes;
}

int compareUnAddrs(void* key1, void* key2, size_t size){
	if(NULL == key1 || NULL == key2 || 0 == size)
		return 1;
	return unAddressCompare(key1, key2)?0:1;
}
int freeUnAddress(void* addr){
	UnIfaceAddr_T* address = (UnIfaceAddr_T*)addr;
	int freeRes = unAddressFree(address);
	free(address);
	return freeRes;
}
int freeNonResolved(void* ngbr){
	NonResolvedNeighbor_T* neighbor = (NonResolvedNeighbor_T*)ngbr;
	int freeRes = unAddressFree(&(neighbor->Address));
	free(neighbor);
	return freeRes;
}
int neighborsInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = hashInit(&(layer->Neighbors), hashChannelAddress, NEIGHBORS_TABLE_SIZE, sizeof(ChannelAddr_T),sizeof(ChannelNeighbor_T));
	if(FUNC_RESULT_SUCCESS != res)
		return res;
	int backRes = hashInit(&(layer->NeighborsBackTranslation), unAddressHash, NEIGHBORS_BACK_TABLE_SIZE, sizeof(UnIfaceAddr_T),sizeof(ChannelAddr_T));
	if(FUNC_RESULT_SUCCESS != backRes)
		return backRes;
	layer->NeighborsBackTranslation.EqualFunction = compareUnAddrs;
	layer->NeighborsBackTranslation.KeyFreeFunction = freeUnAddress;
	int nonRes = hashInit(&(layer->NonResolvedNeighbors), unAddressHash, NEIGHBORS_NONRES_TABLE_SIZE, sizeof(UnIfaceAddr_T),sizeof(NonResolvedNeighbor_T));
	if(FUNC_RESULT_SUCCESS != nonRes)
		return backRes;
	layer->NonResolvedNeighbors.EqualFunction = compareUnAddrs;
	layer->NonResolvedNeighbors.DataFreeFunction = freeNonResolved;
	return FUNC_RESULT_SUCCESS;
}

int neighborsTableDeinit(ChannelLayer_T* layer) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int freeRes = hashFree(&(layer->Neighbors));
	return freeRes;
}
int neighborsBacktableDeinit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int clearRes = hashClear(&(layer->NeighborsBackTranslation));
	if(FUNC_RESULT_SUCCESS != clearRes)
		return clearRes;
	int freeRes = hashFree(&(layer->NeighborsBackTranslation));
	return freeRes;
}
int neighborsNonResDeinit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int clearRes = hashClear(&(layer->NonResolvedNeighbors));
	if(FUNC_RESULT_SUCCESS != clearRes)
		return clearRes;
	int freeRes = hashFree(&(layer->NonResolvedNeighbors));
	return freeRes;
}
int neighborsDeinit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int tableRes = neighborsTableDeinit(layer);
	if(FUNC_RESULT_SUCCESS != tableRes)
		return tableRes;
	int backtableRes = neighborsBacktableDeinit(layer);
	if(FUNC_RESULT_SUCCESS != backtableRes)
		return backtableRes;
	int nonRes = neighborsNonResDeinit(layer);
	if(FUNC_RESULT_SUCCESS != nonRes)
		return nonRes;
	return FUNC_RESULT_SUCCESS;
}

ChannelNeighbor_T* neighborFind(ChannelLayer_T* layer, ChannelAddr_T* address){
	if(NULL == layer)
		return NULL;
	if(NULL == address)
		return NULL;

	ChannelNeighbor_T* neighbor = (ChannelNeighbor_T*)hashGetPtr(&(layer->Neighbors),address);
	return neighbor;
}
RemoteInterface_T* neighborIfaceFind(ChannelNeighbor_T* neighbor){
	if(NULL == neighbor)
		return NULL;

	return NULL;
}
RemoteInterface_T* neighborIfaceFindAddr(ChannelNeighbor_T* neighbor, UnIfaceAddr_T* remoteAddress){
	if(NULL == neighbor)
		return NULL;

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
	int removeRes = neighborNonResRemove(layer, remoteAddress);
	// find neighbor
	ChannelNeighbor_T* neighbor = neighborFind(layer, address);
	if(NULL == neighbor){
		// add new
		isNew = true;
	}
	//search interface in neighbor interfaces table
	RemoteInterface_T* remoteInterface = neighborIfaceFindAddr(neighbor, remoteAddress);
	//if not found
	if(NULL == remoteInterface) {
		// add interface here
		//copy address
		// add to list
		if (isNew) {
			int addNeighborRes = hashAdd(&(layer->Neighbors), address, neighbor);
			free(neighbor); // stored in hash table
			// inform routing
			notifyRouting(layer,LayerCommandType_LostNeighbor, address);
		}
	}
	return FUNC_RESULT_SUCCESS;
}
int neighborRemove(ChannelLayer_T* layer, UnIfaceAddr_T* remoteAddress){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RemoteInterface_T refInterface;
	refInterface.Address = *remoteAddress;
	int cleanRes = 0;
	return cleanRes;
}
int neighborsRemoveAssociated(ChannelLayer_T* layer, int localSocket){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 >= localSocket)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RemoteInterface_T refInterface;
	refInterface.BridgeInterface = interfaceFind(layer,localSocket);
	if(NULL == refInterface.BridgeInterface)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int cleanRes = 0;
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
		if(FUNC_RESULT_SUCCESS != addrRes){
			free(neighbor);
			return addrRes;
		}
		neighbor->SendAttempts = 0;
		neighbor->NextProcessingTime = timeGetCurrent();
		neighbor->LocalInterfaceSocket = localSocket;
		int addRes = hashAdd(&(layer->NonResolvedNeighbors), &(neighbor->Address), neighbor);
		if(FUNC_RESULT_SUCCESS != addRes) {
			unAddressFree(&(neighbor->Address));
			free(neighbor);
			return addRes;
		}
		free(neighbor);
	}
	return FUNC_RESULT_SUCCESS;
}
int neighborNonResProcess(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	moarTime_T currentTime = timeGetCurrent();
	hashIterator_T iterator = {0};
	hashIterator(&(layer->NonResolvedNeighbors), &iterator);
	while(!hashIteratorIsLast(&iterator)){
		NonResolvedNeighbor_T* neighbor = (NonResolvedNeighbor_T*)hashIteratorData(&iterator);
		int compRes = timeCompare(currentTime, neighbor->NextProcessingTime);
		if(compRes >= 0){
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
		hashIteratorNext(&iterator);
	}
	return FUNC_RESULT_SUCCESS;
}
int neighborNonResRemove(ChannelLayer_T* layer, UnIfaceAddr_T* address){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL == address)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = hashRemove(&(layer->NonResolvedNeighbors), address);
	return res;
}
NonResolvedNeighbor_T* neighborNonResFind(ChannelLayer_T* layer, UnIfaceAddr_T* address){
	if(NULL == layer)
		return NULL;
	if(NULL == address)
		return NULL;
	NonResolvedNeighbor_T* neighbor = hashGetPtr(&(layer->NonResolvedNeighbors),address);
	return neighbor;
}