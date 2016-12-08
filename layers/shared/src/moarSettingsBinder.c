//
// Created by svalov on 12/8/16.
//

#include "moarSettingsBinder.h"
#include "funcResults.h"
#include "stddef.h"

void bindingFreeName(SettingsBind_T* binding){
	if(binding != NULL)
		free(binding->Name);
}

int bindingMake(SettingsBind_T* binding, char* name, void* offset, FieldType_T type){
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