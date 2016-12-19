//
// Created by svalov on 12/10/16.
//

#include "moarCommonSettings.h"
#include <funcResults.h>

int makeAddressBinding(SettingsBind_T** binding, int* count){
	if(NULL == binding || NULL == count)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*count = 1;

	SettingsBind_T* bind = malloc((*count)*sizeof(SettingsBind_T));
	if(NULL == bind)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	*binding = bind;

	int res = BINDINGMAKE(bind++, nodeAddress, Address, FieldType_RouteAddr_T); 
	CHECK_RESULT(res);


	return FUNC_RESULT_SUCCESS;
}
int makeIfaceSockBinding(SettingsBind_T** binding, int* count){
	if(NULL == binding || NULL == count)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*count = 1;

	SettingsBind_T* bind = malloc((*count)*sizeof(SettingsBind_T));
	if(NULL == bind)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	*binding = bind;

	int res = BINDINGMAKE(bind++, ifaceSocket, FileName, FieldType_char);
	CHECK_RESULT(res);


	return FUNC_RESULT_SUCCESS;
}
int makeServSockBinding(SettingsBind_T** binding, int* count){
	if(NULL == binding || NULL == count)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*count = 1;

	SettingsBind_T* bind = malloc((*count)*sizeof(SettingsBind_T));
	if(NULL == bind)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	*binding = bind;

	int res = BINDINGMAKE(bind++, serviceSocket, FileName, FieldType_char);
	CHECK_RESULT(res);

	return FUNC_RESULT_SUCCESS;
}
int makeLibraryLocationBinding(SettingsBind_T** binding, int* count){
	if(NULL == binding || NULL == count)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*count = 1;

	SettingsBind_T* bind = malloc((*count)*sizeof(SettingsBind_T));
	if(NULL == bind)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	*binding = bind;

	int res = BINDINGMAKE(bind++, libraryLocation, FileName, FieldType_char);
	CHECK_RESULT(res);

	return FUNC_RESULT_SUCCESS;
}