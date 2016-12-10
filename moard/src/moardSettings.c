//
// Created by svalov on 12/9/16.
//

#include "moardSettings.h"
#include <moarSettingsBinder.h>
#include <moarConfigReader.h>
#include <funcResults.h>
#include <stdio.h>

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

int settingsLoad(moardSettings* settings,  char* fileName, hashTable_T* table){
	if(NULL == settings || NULL == fileName || NULL == table)
		return FUNC_RESULT_FAILED_ARGUMENT;

	int confPrepareRes = configInit(table);
	if (FUNC_RESULT_SUCCESS != confPrepareRes) {
		fprintf(stderr, "Can not init config storage\r\n");
		return FUNC_RESULT_FAILED;
	}

	int confRes = configRead(table, fileName);
	if (FUNC_RESULT_SUCCESS != confRes) {
		fprintf(stderr, "Can not read core config file %s\r\n", fileName);
		return FUNC_RESULT_FAILED;
	}

	int count = 0;
	SettingsBind_T* binding = NULL;
	int bindRes = makeMoardSettingsBinding(&binding, &count);
	CHECK_RESULT(bindRes);

	int settingsBindRes = bindingBindStruct(table,binding, count, settings);
	CHECK_RESULT(settingsBindRes);

	free(binding);
	return FUNC_RESULT_SUCCESS;
}