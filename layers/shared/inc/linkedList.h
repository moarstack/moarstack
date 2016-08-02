//
// Created by svalov on 7/26/16.
//

#ifndef MOARSTACK_LINKEDLIST_H
#define MOARSTACK_LINKEDLIST_H

typedef struct LinkedListItem_T LinkedListItem_T;

struct LinkedListItem_T{
	LinkedListItem_T* Next;
	LinkedListItem_T* Prev;
	void * Data;
};
#ifdef __cplusplus
extern "C" {
#endif


extern int CreateList(LinkedListItem_T *first);

extern LinkedListItem_T* DeleteElement(LinkedListItem_T *item);

extern LinkedListItem_T* NextElement(LinkedListItem_T *current);

extern LinkedListItem_T* PrevElement(LinkedListItem_T *current);

extern int AddNext(LinkedListItem_T *item, void *data);

#ifdef __cplusplus
}
#endif



#endif //MOARSTACK_LINKEDLIST_H
