//
// Created by svalov on 12/8/16.
//

#include "moarSettingsBinder.h"
#include "funcResults.h"
#include "stddef.h"
#include <errno.h>
#include <moarCommons.h>
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>

void bindingFreeName(SettingsBind_T* binding){
	if(binding != NULL)
		free(binding->Name);
}

int bindingMake(SettingsBind_T* binding, char* name, Offset_T offset, FieldType_T type){
	if(NULL == binding || NULL == name)
		return FUNC_RESULT_FAILED_ARGUMENT;

	binding->Offset = offset;
	binding->FieldType = type;

	char* newLine = mStrDup(name);
	binding->Name = newLine;
	// to lower
	for(;*newLine;newLine++)
		*newLine = (char)tolower(*newLine);
	return FUNC_RESULT_SUCCESS;
}

int bindingSet_int(void* ptr, char* val){
	int value = 0;
	int res = sscanf(val, "%d", &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((int*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_char(void* ptr, char* val){
	*((char**)ptr) = val;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_uint64_t(void* ptr, char* val){
	uint64_t value = 0;
	int res = sscanf(val, "%" SCNu64, &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((uint64_t*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_uint32_t(void* ptr, char* val){
	uint32_t value = 0;
	int res = sscanf(val, "%" SCNu32, &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((uint32_t*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_uint16_t(void* ptr, char* val){
	uint16_t value = 0;
	int res = sscanf(val, "%" SCNu16, &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((uint16_t*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_uint8_t(void* ptr, char* val){
	uint8_t value = 0;
	int res = sscanf(val, "%" SCNu8, &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((uint8_t*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_int64_t(void* ptr, char* val){
	int64_t value = 0;
	int res = sscanf(val, "%" SCNd64, &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((int64_t*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_int32_t(void* ptr, char* val){
	int32_t value = 0;
	int res = sscanf(val, "%" SCNd32, &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((int32_t*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_int16_t(void* ptr, char* val){
	int16_t value = 0;
	int res = sscanf(val, "%" SCNd16, &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((int16_t*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}
int bindingSet_int8_t(void* ptr, char* val){
	int8_t value = 0;
	int res = sscanf(val, "%" SCNd8, &value);
	if(1 != res)
		return FUNC_RESULT_FAILED_ARGUMENT;
	*((int8_t*)ptr) = value;
	return FUNC_RESULT_SUCCESS;
}

int bindingBind(SettingsBind_T* binding, void* targetStruct, char* val){
	if(NULL == binding || NULL == val || NULL == targetStruct)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = FUNC_RESULT_FAILED;
	void* ptr = (uint8_t *)targetStruct + binding->Offset;
	switch(binding->FieldType)
	{
		case FieldType_uint64_t:
			res = bindingSet_uint64_t(ptr, val);
			break;
		case FieldType_uint32_t:
			res = bindingSet_uint32_t(ptr, val);
			break;
		case FieldType_uint16_t:
			res = bindingSet_uint16_t(ptr, val);
			break;
		case FieldType_uint8_t:
			res = bindingSet_uint8_t(ptr, val);
			break;
		case FieldType_int64_t:
			res = bindingSet_int64_t(ptr, val);
			break;
		case FieldType_int32_t:
			res = bindingSet_int32_t(ptr, val);
			break;
		case FieldType_int16_t:
			res = bindingSet_int16_t(ptr, val);
			break;
		case FieldType_int8_t:
			res = bindingSet_int8_t(ptr, val);
			break;
		case FieldType_int:
			res = bindingSet_int(ptr, val);
			break;
		case FieldType_char:
			res = bindingSet_char(ptr, val);
			break;
		default:
			res = FUNC_RESULT_FAILED_ARGUMENT;
	}
	return res;
}
int bindingBindStruct(hashTable_T* settings, SettingsBind_T* binding, int bindCount, void* targetStruct){
	if(NULL == settings || NULL == binding || NULL == targetStruct)
		return FUNC_RESULT_FAILED_ARGUMENT;
	for(int i=0; i<bindCount; i++){
		//get line from table
		char* val;
		int res = hashGet(settings, &(binding[i].Name), &val);
		if(res == FUNC_RESULT_SUCCESS){
			// process found
			res = bindingBind(binding+i, targetStruct, val);
			CHECK_RESULT(res);
		}
	}
	return FUNC_RESULT_SUCCESS;
}