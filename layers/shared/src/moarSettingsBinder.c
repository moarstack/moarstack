//
// Created by svalov on 12/8/16.
//

#include "moarSettingsBinder.h"
#include "funcResults.h"
#include "stddef.h"
#include <errno.h>

void bindingFreeName(SettingsBind_T* binding){
	if(binding != NULL)
		free(binding->Name);
}

int bindingMake(SettingsBind_T* binding, char* name, Offset_T offset, FieldType_T type){
	if(NULL == binding || NULL == name)
		return FUNC_RESULT_FAILED_ARGUMENT;

	binding->Offset = offset;
	binding->FieldType = type;

	char* newLine = strdup(name);
	binding->Name = newLine;
	// to lower
	for(;*newLine;newLine++)
		*newLine = (char)tolower(*newLine);
	return FUNC_RESULT_SUCCESS;
}

int bindingSetInt(void* ptr, char* val){
	int value = 0;
	int res = sscanf(val, "%d", &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((int*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}

int bindingBind(SettingsBind_T* binding, void* targetStruct, char* val){
	if(NULL == binding || NULL == val || NULL == targetStruct)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	void* ptr = targetStruct+binding->Offset;
	switch(binding->FieldType)
	{
		case FieldType_int:
			res = bindingSetInt(ptr, val);
			break;
		default:
			res = FUNC_RESULT_FAILED_ARGUMENT;
	}
	return res;
}