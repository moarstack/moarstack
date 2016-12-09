//
// Created by svalov on 12/9/16.
//

#include "moardSettings.h"
#include <moarSettingsBinder.h>
#include <moarConfigReader.h>
#include <funcResults.h>

int makeMoardSettingsBinding(SettingsBind_T** binding, int* count){
	if(NULL == binding || NULL == count)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*count = 2;

	SettingsBind_T* bind = malloc((*count)*sizeof(SettingsBind_T));
	if(NULL == bind)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	*binding = bind;

	CHECK_RESULT(BINDINGMAKE(bind++, moardSettings, LogPath, FieldType_char));
	CHECK_RESULT(BINDINGMAKE(bind++, moardSettings, LayersEnabledDir, FieldType_char));

	return FUNC_RESULT_SUCCESS;
}
