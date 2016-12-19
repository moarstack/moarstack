//
// Created by svalov on 7/27/16.
//

#include <moarChannelPrivate.h>
#include <moarChannelInterfaces.h>
#include <moarChannelHello.h>
#include <moarChannelNeighbors.h>

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

int compareUnAddrs(const void* key1, const void* key2, size_t size){
	if(NULL == key1 || NULL == key2 || 0 == size)
		return 1;
	return unAddressCompare((UnIfaceAddr_T*)key1, (UnIfaceAddr_T*)key2)?0:1;
}
void freeUnAddress(void* addr){
	UnIfaceAddr_T* address = (UnIfaceAddr_T*)addr;
	unAddressFree(address);
	free(address);
}
void freeNonResolved(void* ngbr){
	NonResolvedNeighbor_T* neighbor = (NonResolvedNeighbor_T*)ngbr;
	unAddressFree(&(neighbor->Address));
	free(neighbor);
}
void freeChannelNeighbor(void* ngbr){
	ChannelNeighbor_T* neighbor = (ChannelNeighbor_T*)ngbr;
	hashClear(&(neighbor->Interfaces));
	hashFree(&(neighbor->Interfaces));
	free(neighbor);
}

int neighborsInit(ChannelLayer_T* layer){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = hashInit(&(layer->Neighbors), hashChannelAddress, NEIGHBORS_TABLE_SIZE, sizeof(ChannelAddr_T),sizeof(ChannelNeighbor_T));
	if(FUNC_RESULT_SUCCESS != res)
		return res;
	layer->Neighbors.DataFreeFunction = freeChannelNeighbor;

	int backRes = hashInit(&(layer->NeighborsBackTranslation), unAddressHash, NEIGHBORS_BACK_TABLE_SIZE, sizeof(UnIfaceAddr_T),sizeof(ChannelAddr_T));
	if(FUNC_RESULT_SUCCESS != backRes)
		return backRes;
	layer->NeighborsBackTranslation.EqualFunction = compareUnAddrs;
	layer->NeighborsBackTranslation.KeyFreeFunction = freeUnAddress;

	int nonRes = hashInit(&(layer->NeighborsNonResolved), unAddressHash, NEIGHBORS_NONRES_TABLE_SIZE, sizeof(UnIfaceAddr_T),sizeof(NonResolvedNeighbor_T));
	if(FUNC_RESULT_SUCCESS != nonRes)
		return backRes;
	layer->NeighborsNonResolved.EqualFunction = compareUnAddrs;
	layer->NeighborsNonResolved.DataFreeFunction = freeNonResolved;
	return FUNC_RESULT_SUCCESS;
}

int neighborsTableDeinit(ChannelLayer_T* layer) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int clearRes = hashClear(&(layer->Neighbors));
	if(FUNC_RESULT_SUCCESS != clearRes)
		return clearRes;
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
	int clearRes = hashClear(&(layer->NeighborsNonResolved));
	if(FUNC_RESULT_SUCCESS != clearRes)
		return clearRes;
	int freeRes = hashFree(&(layer->NeighborsNonResolved));
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
	hashIterator_T iterator = {0};
	hashIterator(&(neighbor->Interfaces),&iterator);
	while(!hashIteratorEnded( &iterator )){
		RemoteInterface_T* iface = (RemoteInterface_T*)hashIteratorData(&iterator);
		if(NULL != iface->BridgeInterface && iface->BridgeInterface->Ready)
			return iface;
		hashIteratorNext(&iterator);
	}
	return NULL;
}
RemoteInterface_T* neighborIfaceFindAddr(ChannelNeighbor_T* neighbor, UnIfaceAddr_T* remoteAddress){
	if(NULL == neighbor)
		return NULL;
	RemoteInterface_T* iface = (RemoteInterface_T*)hashGetPtr(&(neighbor->Interfaces), remoteAddress);
	return iface;
}
int neighborAdd(ChannelLayer_T* layer, ChannelAddr_T* address, UnIfaceAddr_T* remoteAddress, int localSocket) {
	if (NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (NULL == remoteAddress)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if (0 >= localSocket)
		return FUNC_RESULT_FAILED_ARGUMENT;

	//if channel address is unknown
	if (NULL == address) {
		int res = neighborNonResAdd(layer, remoteAddress, localSocket);
		return res;
	}
	bool isNew = false;
	//find unresolved
	int removeRes = neighborNonResRemove(layer, remoteAddress);
	// find neighbor
	ChannelNeighbor_T *neighbor = neighborFind(layer, address);
	if (NULL == neighbor) {
		// add new
		neighbor = (ChannelNeighbor_T *) malloc(sizeof(ChannelNeighbor_T));
		if (NULL == neighbor)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		neighbor->RemoteAddress = *address;
		//init interfaces store
		int res = hashInit(&(neighbor->Interfaces), unAddressHash,
						   NEIGHBORS_INTERFACES_TABLE_SIZE, sizeof(UnIfaceAddr_T), sizeof(RemoteInterface_T));
		neighbor->Interfaces.EqualFunction = compareUnAddrs;
		neighbor->Interfaces.KeyFreeFunction = freeUnAddress;
		if (FUNC_RESULT_SUCCESS != res) {
			free(neighbor);
			return res;
		}
		isNew = true;
	}
	//search interface in neighbor interfaces table
	RemoteInterface_T *remoteInterface = neighborIfaceFindAddr(neighbor, remoteAddress);
	//if not found
	if (NULL == remoteInterface) {
		// add interface here
		remoteInterface = (RemoteInterface_T *) malloc(sizeof(RemoteInterface_T));
		if (NULL != remoteInterface) {
			//copy address
			int cloneRes = unAddressClone(remoteAddress, &(remoteInterface->Address));
			//add descriptor pointer
			remoteInterface->BridgeInterface = interfaceFind(layer, localSocket);
			// add to back translation
			UnIfaceAddr_T addr = {0};
			int internalCloneRes = unAddressClone(remoteAddress, &(addr));

			if(FUNC_RESULT_SUCCESS == cloneRes && FUNC_RESULT_SUCCESS == internalCloneRes) {
				// add to back translation
				int backAddRes = hashAdd(&(layer->NeighborsBackTranslation), &addr, address);
				// add to list
				int addRes = hashAdd(&(neighbor->Interfaces), &(remoteInterface->Address), remoteInterface);
				//remove if failed
				if(FUNC_RESULT_SUCCESS != addRes || FUNC_RESULT_SUCCESS != backAddRes){
					hashRemove(&(layer->NeighborsBackTranslation), &addr);
					hashRemove(&(neighbor->Interfaces), &(remoteInterface->Address));
				}
			}
			//free remote
			free(remoteInterface);
		}
	}
	if (isNew) {
		if (neighbor->Interfaces.Count != 0) {
			int addNeighborRes = hashAdd(&(layer->Neighbors), address, neighbor);
			// inform routing
			if (FUNC_RESULT_SUCCESS == addNeighborRes)
				notifyRouting(layer, LayerCommandType_LostNeighbor, address);
		}
		free(neighbor); // stored in hash table
	}
	return FUNC_RESULT_SUCCESS;
}
int neighborRemove(ChannelLayer_T* layer, UnIfaceAddr_T* remoteAddress){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// get channel address by back translation
	ChannelAddr_T* address = (ChannelAddr_T*)hashGetPtr(&(layer->NeighborsBackTranslation),remoteAddress);
	if(NULL == address)
		return FUNC_RESULT_SUCCESS;
	ChannelNeighbor_T* neighbor = (ChannelNeighbor_T*)hashGetPtr(&(layer->Neighbors),address);
	if(NULL == neighbor)
		return FUNC_RESULT_SUCCESS;
	// remove from channel neighbor interfaces
	int removeRemoteRes = hashRemove(&(neighbor->Interfaces), remoteAddress);
	// if no more - remove channel neighbor
	if(0 == neighbor->Interfaces.Count){
		int removeNeighborRes = hashRemove(&(layer->Neighbors),address);
		// and notify
		notifyRouting(layer,LayerCommandType_LostNeighbor, address);
	}
	// remove from back translation
	int removeBackRes = hashRemove(&(layer->NeighborsBackTranslation), remoteAddress);
	return FUNC_RESULT_SUCCESS;
}
int neighborsRemoveAssociated(ChannelLayer_T* layer, int localSocket){
	if(NULL == layer)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(0 >= localSocket)
		return FUNC_RESULT_FAILED_ARGUMENT;

	//check for correct local socket
	InterfaceDescriptor_T* localIface = interfaceFind(layer,localSocket);
	if(NULL == localIface)
		return FUNC_RESULT_FAILED_ARGUMENT;

	hashIterator_T neighborsIterator = {0};
	hashIterator(&(layer->Neighbors), &neighborsIterator);
	while(!hashIteratorEnded( &neighborsIterator )){
		ChannelNeighbor_T* neighbor = (ChannelNeighbor_T*)hashIteratorData(&neighborsIterator);
		//foreach interface
		hashIterator_T interfaceIterator = {0};
		hashIterator(&(neighbor->Interfaces), &interfaceIterator);
		while(!hashIteratorEnded( &interfaceIterator )){
			RemoteInterface_T* remoteInterface = (RemoteInterface_T*)hashIteratorData(&interfaceIterator);
			//check socket
			if(NULL != remoteInterface && NULL != remoteInterface->BridgeInterface
			   && localSocket == remoteInterface->BridgeInterface->Socket)
			{
				//remove
				// remove from channel neighbor interfaces
				int removeRemoteRes = hashRemove(&(neighbor->Interfaces), &(remoteInterface->Address));
				int removeBackRes = hashRemove(&(layer->NeighborsBackTranslation), &(remoteInterface->Address));
			}
			hashIteratorNext(&interfaceIterator);
		}
		if(0 == neighbor->Interfaces.Count){
			int removeNeighborRes = hashRemove(&(layer->Neighbors), &neighbor->RemoteAddress);
			ChannelAddr_T addr = neighbor->RemoteAddress;
			// and notify
			notifyRouting(layer,LayerCommandType_LostNeighbor, &addr);
		}
		//check for interfaces count
		hashIteratorNext(&neighborsIterator);
	}

	return FUNC_RESULT_SUCCESS;
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
		int addRes = hashAdd(&(layer->NeighborsNonResolved), &(neighbor->Address), neighbor);
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
	hashIterator(&(layer->NeighborsNonResolved), &iterator);
	while(!hashIteratorEnded( &iterator )){
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
	int res = hashRemove(&(layer->NeighborsNonResolved), address);
	return res;
}
NonResolvedNeighbor_T* neighborNonResFind(ChannelLayer_T* layer, UnIfaceAddr_T* address){
	if(NULL == layer)
		return NULL;
	if(NULL == address)
		return NULL;
	NonResolvedNeighbor_T* neighbor = hashGetPtr(&(layer->NeighborsNonResolved),address);
	return neighbor;
}