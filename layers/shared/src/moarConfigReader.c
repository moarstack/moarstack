//
// Created by svalov on 12/6/16.
//

#include "moarConfigReader.h"
#include <funcResults.h>
#include <hashFunc.h>
#include <hashTable.h>
#include <string.h>
#include <unistd.h>

int equalString(const void* key1, const void* key2, size_t size){
	if(NULL == key1 || NULL == key2)
		return 1;
	return strcmp(*(char**)key1, *(char**)key2);
}

void freeString(void* data){
	if(NULL!=data) {
		free(*(char **) data);
		free(data);
	}
}

hashVal_T hashString(void* data, size_t size){
	size_t len = strlen(*(char**)data);
	return hashBytesEx(*(char**)data, len, 0xF4E617);
}

int configPrepare(hashTable_T* config){
	if(NULL == config)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int initRes = hashInit(config, hashString, CONFIG_SIZE, sizeof(char*), sizeof(char*));
	if(FUNC_RESULT_SUCCESS != initRes)
		return initRes;
	config->DataFreeFunction = freeString;
	config->KeyFreeFunction = freeString;
	config->EqualFunction = equalString;
	return FUNC_RESULT_SUCCESS;
}

bool isDelimeter(char p){
	if(p == ' ' || p == '\t' || p == '=' || p=='\r' || p=='\n')
		return true;
	return false;
}
bool isEol(char p){
	if(p=='\r' || p=='\n')
		return true;
	return false;
}

void trim(char* s){
	char* p = s;
	size_t l = strlen(s);
	while(isDelimeter(p[l-1])) p[--l] = 0;
	while(*p && isDelimeter(*p)) ++p, --l;
	memmove(s,p,l+1);
}

int processLine(hashTable_T* config, char* line){
	if(NULL == config || NULL == line)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//some line here
	if(line[0] != '#' && line[0] != '\0') {
		//process key
		char *end = line;
		while (*end && !isDelimeter(*end)) end++;
		// key now from buffer to end
		char *key = malloc(end - line + 1);
		if(NULL == key)
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		//copy
		memcpy(key, line, end - line);
		key[end - line] = '\0';
		//ifnore some stuff
		while (*end && isDelimeter(*end)) end++;
		char *value = end;
		// process value
		while (*end && !isEol(*end)) end++;
		// value from value to end
		char *val = malloc(end - value + 1);
		if(NULL == val) {
			free(key);
			return FUNC_RESULT_FAILED_MEM_ALLOCATION;
		}
		//copy
		memcpy(val, value, end - value);
		value[end - value] = '\0';
		// add here
		int res = hashAdd(config, &key, &val);
		if(res != FUNC_RESULT_SUCCESS){
			//cleanup
			free(key);
			free(val);
		}
		return res;
	}
	return FUNC_RESULT_SUCCESS;
}

int configRead(hashTable_T* config, char* fileName){
	if(NULL == fileName || NULL == config)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//check existence of fileName
	int accRes = access(fileName, F_OK);
	if(accRes == -1)
		return FUNC_RESULT_FAILED_IO;
	// open
	FILE* configFile = fopen(fileName, "r");
	if(!configFile)
	{
		fclose(configFile);
		return FUNC_RESULT_FAILED_IO;
	}
	char buffer[255];
	int i=0;
	int res = FUNC_RESULT_SUCCESS;
	//read by line
	while(fgets(buffer, sizeof(buffer), configFile) != NULL){
			trim(buffer);
			int res = processLine(config, buffer);
			if(res!= FUNC_RESULT_SUCCESS)
				break;
	}
	//close
	fclose(configFile);
	return res;
}