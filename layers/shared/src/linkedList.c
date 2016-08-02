//
// Created by svalov on 7/26/16.
//

#include "linkedList.h"
#include <funcResults.h>
#include <stddef.h>
#include <stdlib.h>

int CreateList(LinkedListItem_T* first){
	if(NULL == first)
		return FUNC_RESULT_FAILED_ARGUMENT;
	first->Data = NULL;
	first->Next = first;
	first->Prev = first;
	return FUNC_RESULT_SUCCESS;
}
LinkedListItem_T* DeleteElement(LinkedListItem_T* item){
	if(NULL == item)
		return NULL;
	if(NULL == item->Data)
		return NULL;
	LinkedListItem_T* res = item->Prev;
	//add pointers here
	res->Next = item->Next;
	item->Next->Prev = res;
	free(item);
	return res;
}
LinkedListItem_T* NextElement(LinkedListItem_T* current){
	if(NULL == current)
		return NULL;
	if(NULL == current->Next->Data)
		return NULL;
	return current->Next;
}
LinkedListItem_T* PrevElement(LinkedListItem_T* current){
	if(NULL == current)
		return NULL;
	if(NULL == current->Prev->Data)
		return NULL;
	return current->Prev;
}
int AddNext(LinkedListItem_T* item, void* data){
	if(NULL == item)
		return FUNC_RESULT_FAILED_ARGUMENT;
	if(NULL != data){
		//add
		LinkedListItem_T* newItem = (LinkedListItem_T*)malloc(sizeof(LinkedListItem_T));
		if(NULL == newItem)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		newItem->Data = data;
		newItem->Next = item->Next;
		newItem->Prev = item;
		item->Next->Prev = newItem;
		item->Next = newItem;
	}
	return FUNC_RESULT_SUCCESS;
}