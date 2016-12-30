//
// Created by svalov on 12/12/16.
//

#include "mockIfaceSettings.h"

int makeMockIfaceBinding(SettingsBind_T** binding, int* count){
	if(NULL == binding || NULL == count)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*count = 4;

	SettingsBind_T* bind = malloc((*count)*sizeof(SettingsBind_T));
	if(NULL == bind)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	*binding = bind;

	BINDINGMAKE(bind++, mockIface, LogPath, FieldType_char);
	BINDINGMAKE(bind++, mockIface, MockItSocket, FieldType_char);
	BINDINGMAKE(bind++, mockIface, Address, FieldType_int);
	BINDINGMAKE(bind++, mockIface, Sensitivity, FieldType_float);

	return FUNC_RESULT_SUCCESS;
}