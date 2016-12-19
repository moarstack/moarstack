//
// Created by svalov on 12/6/16.
//

#include "moarConfigReader.h"
#include <funcResults.h>
#include <hashFunc.h>
#include <hashTable.h>
#include <string.h>
#include <unistd.h>
#include <moarCommons.h>
#include <bits/string2.h>
#include <ctype.h>

int equalString(const void* key1, const void* key2, size_t size){
	if(NULL == key1 || NULL == key2)
		return 1;
	return strcmp(STRVAL(key1), STRVAL(key2));
}

void freeString(void* data){
	if(NULL!=data) {
		free(*(char **) data);
		free(data);
	}
}

hashVal_T hashString(void* data, size_t size){
	size_t len = strlen(STRVAL(data));
	return hashBytesEx(STRVAL(data), len, 0xF4E617);
}

int configInit(hashTable_T* config){
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
int configFree(hashTable_T* config){
	if(NULL == config)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int res = hashClear(config);
	CHECK_RESULT(res);

	res = hashFree(config);
	CHECK_RESULT(res);

	return FUNC_RESULT_SUCCESS;
}

bool isEol(char p){
	if(p=='\r' || p=='\n')
		return true;
	return false;
}
bool isSpace(char p){
	if(p == ' ' || p == '\t' || isEol(p))
		return true;
	return false;
}
bool isDelimeter(char p){
	if(isSpace(p) || p == '=' )
		return true;
	return false;
}
void trim(char* s){
	char* p = s;
	size_t l = strlen(s);
	while(isSpace(p[l-1])) p[--l] = 0;
	while(*p && isSpace(*p)) ++p, --l;
	memmove(s,p,l+1);
}

int extractKey(char* line, char** key, char** pos){
	//process key
	char *end = line;
	// search end of key
	while (*end && !isDelimeter(*end))
		end++;
	//if no key
	if(0 == end-line)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// key now from buffer to end
	*key = malloc(end - line + 1);
	if(NULL == *key)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	//copy with lowering
	for(char* p = *key, *l = line; l < end; p++, l++)
		*p =  (char) tolower(*l);
	(*key)[end - line] = '\0';
	*pos = end;
	return FUNC_RESULT_SUCCESS;
}
int extractValue(char* value, char** val){
	// begin of value
	char* end = value;
	// search end of value
	while (*end && !isEol(*end))
		end++;
	//if no value
	if(0 == end-value)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// value from value to end
	*val = malloc(end - value + 1);
	if(NULL == *val)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;
	//copy
	memcpy(*val, value, end - value);
	(*val)[end - value] = '\0';
	return FUNC_RESULT_SUCCESS;
}

int configProcessLine(hashTable_T* config, char* line){
	if(NULL == config || NULL == line)
		return FUNC_RESULT_FAILED_ARGUMENT;
	// trim some trash
	trim(line);
	//some line here
	if(line[0] != '#' && line[0] != '\0') {

		char* pos;
		char* key = NULL;
		int keyRes = extractKey(line, &key, &pos);
		if(keyRes!= FUNC_RESULT_SUCCESS)
			return FUNC_RESULT_SUCCESS;

		//ignore some stuff between key and value
		while (*pos && isDelimeter(*pos))
			pos++;

		char* val = NULL;
		int valRes = extractValue(pos, &val);
		if(FUNC_RESULT_SUCCESS != valRes){
			free(key);
			return FUNC_RESULT_SUCCESS;
		}
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
		return FUNC_RESULT_FAILED_IO;
	//read
	char buffer[LINE_SIZE];
	int res = FUNC_RESULT_SUCCESS;
	//read by line
	while(fgets(buffer, sizeof(buffer), configFile) != NULL){
			int res = configProcessLine(config, buffer);
			if(res!= FUNC_RESULT_SUCCESS)
				break;
	}
	//close
	fclose(configFile);
	return res;
}

int configMerge(hashTable_T* dest, hashTable_T* source){
	if(NULL == dest || NULL == source)
		return FUNC_RESULT_FAILED_ARGUMENT;
		hashIterator_T	iter = {0};
	int res = hashIterator(source, &iter);
	CHECK_RESULT(res);
	while(!hashIteratorEnded( &iter )){

		char* iterKey = *((char**)hashIteratorKey(&iter));
		char* iterData = *((char**)hashIteratorData(&iter));
		if(NULL != iterKey && NULL != iterData){

			//ignore entry if contain
			bool contain = hashContain(dest, &iterKey);
			if(!contain) {
				//copy two strings
			char* newKey = mStrDup(iterKey);
				if (NULL == newKey)
					return FUNC_RESULT_FAILED_MEM_ALLOCATION;
			char* newData = mStrDup(iterData);
				if (NULL == newData) {
					free(newKey);
					return FUNC_RESULT_FAILED_MEM_ALLOCATION;
				}
				//and add to table
				res = hashAdd(dest, &newKey, &newData);
				CHECK_RESULT(res);
			}
		}
		res = hashIteratorNext(&iter);
		CHECK_RESULT(res);
	}
	hashIteratorFree(&iter);
	return FUNC_RESULT_SUCCESS;
}