//
// Created by svalov on 9/14/16.
//

#ifndef MOARSTACK_MOARROUTINGPACKETSTORAGE_H
#define MOARSTACK_MOARROUTINGPACKETSTORAGE_H

#include <moarRoutingStoredPacket.h>
#include <hashTable.h>
#include <priorityQueue.h>

#define RMID_HASH_SIZE 	37
#define MID_HASH_SIZE 	37
#define DEST_HASH_SIZE 	37
#define QUEUE_SIZE 		32

// structure
typedef struct{
	int 			Count;
	PriorityQueue_T NextProcessingTime;
	hashTable_T		Destinations;
	hashTable_T		MessageIds;
	hashTable_T		RoutingMessageIds;
}PacketStorage_T;

// init
int psInit(PacketStorage_T* storage);
// deinit
int psDeinit(PacketStorage_T* storage);
// add
int psAdd(PacketStorage_T* storage, RouteStoredPacket_T* packet);
// remove
int psRemove(PacketStorage_T* storage, RouteStoredPacket_T* packet);
// search
int psGetDest(PacketStorage_T* storage, RouteAddr_T* dest, RouteStoredPacket_T* packet);
int psGetMid(PacketStorage_T* storage, MessageId_T* mid, RouteStoredPacket_T* packet);
int psGetRmid(PacketStorage_T* storage, RoutingMessageId_T* rmid, RouteStoredPacket_T* packet);

RouteStoredPacket_T* psGetDestPtr(PacketStorage_T* storage, RouteAddr_T* dest);
RouteStoredPacket_T* psGetMidPtr(PacketStorage_T* storage, MessageId_T* mid);
RouteStoredPacket_T* psGetRmidPtr(PacketStorage_T* storage, RoutingMessageId_T* rmid);

// get top
RouteStoredPacket_T* psGetTop(PacketStorage_T* storage);


#endif //MOARSTACK_MOARROUTINGPACKETSTORAGE_H
