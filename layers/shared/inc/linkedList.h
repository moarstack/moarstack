//
// Created by svalov on 7/26/16.
//

#ifndef MOARSTACK_LINKEDLIST_H
#define MOARSTACK_LINKEDLIST_H

#pragma pack(push, 1)

typedef struct LinkedListItem_T LinkedListItem_T;

struct LinkedListItem_T{
	LinkedListItem_T* Next;
	LinkedListItem_T* Prev;
	void * Data;
};

#pragma pack(pop)

extern int CreateList(LinkedListItem_T *first);

extern LinkedListItem_T* DeleteElement(LinkedListItem_T *item);

extern LinkedListItem_T* NextElement(LinkedListItem_T *current);

extern LinkedListItem_T* PrevElement(LinkedListItem_T *current);

extern int AddNext(LinkedListItem_T *item, void *data);

#endif //MOARSTACK_LINKEDLIST_H
