//
// Created by svalov on 7/2/16.
//
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "libraryLoader.h"
#include "configFiles.h"
#include "moarLibInterface.h"	// MoarLayerStartupParams_T
#include "threadManager.h"
#include "moarLibrary.h"
#include "layerSockets.h"		// socketsPrepare(), socketUp(), socketDown() and so on
#include "moarInterface.h"		// MoarIfaceStartupParams_T
#include <moarLogger.h>
#include <errno.h>
#include <moarInterface.h>
#include <moarConfigReader.h>
#include <moardSettings.h>
#include <moarCommonSettings.h>
#include <libgen.h>
#include <dirent.h>
#include <moarLibrary.h>
#include <queue.h>
#include <getopt.h>

#define PATH_SEPARATOR 				"/"

#ifndef LOAD_MULTIPLE_INTERFACES
#define LAYERS_COUNT	(MoarLayer_LayersCount)
#else
#define LAYERS_COUNT	(MoarLayer_LayersCount+1)
#endif

#pragma pack(push, 1)

typedef struct{
	char* ConfigFile;
} MoardCliArgs_T;

#pragma pack(pop)

Queue_T layersRunning = {0};
int layersCount[MoarLayer_LayersCount] = {0};

void signalHandler(int signo){
    if(SIGINT == signo){
        printf("Received SIGINT, terminating\n");
    }
    else
        printf("Received signal %d\n",signo);
}

int runLayer( MoarLibrary_T * layerLibrary, hashTable_T *config) {
	int result;
	MoarLayerType_T layerType = layerLibrary->Info.LayerType;
	MoarLayerStartupParams_T *layerStartupParams;

	layerStartupParams = (MoarLayerStartupParams_T *) calloc(1, sizeof(MoarLayerStartupParams_T));

	if (NULL == layerStartupParams)
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	layerStartupParams->DownSocketHandler = SocketDown(layerType);
	layerStartupParams->UpSocketHandler = SocketUp(layerType);
	layerStartupParams->LayerConfig = config;

	result = createThread(layerLibrary, layerStartupParams);
	printf(FUNC_RESULT_SUCCESS == result ? "%s started\n" : "failed starting %s\n", layerLibrary->Info.LibraryName);
	return result;
}

int loadLayer(MoarLibrary_T* library, char* fileName, hashTable_T* moardConfig, hashTable_T* layerConfig){
	if(NULL == library || NULL == fileName || NULL == moardConfig || NULL == layerConfig)
		return FUNC_RESULT_FAILED_ARGUMENT;
	//init config
	int res = configInit(layerConfig);
	CHECK_RESULT(res);
	// get config
	res = configRead(layerConfig, fileName);
	CHECK_RESULT(res);
	// merge without override
	res = configMerge(layerConfig, moardConfig);
	CHECK_RESULT(res);
	// get file name
	libraryLocation location = {0};
	res = bindingBindStructFunc(layerConfig, makeLibraryLocationBinding, &location);
	CHECK_RESULT(res);
	// load layer
	res = loadLibrary(location.FileName, library);
	return res;
}

int LogWorkIllustration( void ) {
	LogHandle_T log;
	int			result;
	char		bd[] = { '0', '1', '\0', '2', '3', '\0', '4', '5' };
	int 		ibd[] = { 1, 2, 0, 3, 4, 0, 5, 6 };

	result = LogOpen( "/tmp/someMoarLog.log", &log );

	if( FUNC_RESULT_SUCCESS != result ) {
		printf( "Error starting logger\n" );
		fflush( stdout );
		return result;
	}

	result = LogSetLevelLog( log, LogLevel_Debug4 );

	// who cares about the results?..
	result = LogWrite( log, LogLevel_Debug3, "Some%s data of %zu\nlength: %b\n\n(%s)\n", " binary", sizeof( bd ), bd, sizeof( bd ), "Debug3" );
	result = LogWrite( log, LogLevel_Debug4, "Some%s data of %zu\nlength: %b\n\n(%s)\n", " binary", sizeof( bd ), bd, sizeof( bd ), "Debug4" );
	result = LogWrite( log, LogLevel_DebugQuiet, "Some%s data of %zu\nlength: %b\n\n(%s)\n", " binary", sizeof( bd ), bd, sizeof( bd ), "DebugQuiet" );
	result = LogWrite( log, LogLevel_Information, "Some%s data of %zu\nlength: %b\n\n(%s)\n", " binary", sizeof( bd ), bd, sizeof( bd ), "Information" );
	result = LogWrite( log, LogLevel_Warning, "Some%s data of %zu\nlength: %b\n\n(%s)\n", " binary", sizeof( bd ), bd, sizeof( bd ), "Warning" );
	result = LogWrite( log, LogLevel_Warning, "Some%s data of %zu\nlength: %b\n\n(%s)\n", " binary", sizeof( bd ), NULL, sizeof( bd ), "NULL binary" );
	result = LogWrite( log, LogLevel_Warning, "Some%s data of %zu\nlength: %b\n\n(%s)\n", " binary", 0, bd, 0, "zero size" );
	result = LogWrite( log, LogLevel_Warning, "Some%s data of %zu\nlength: %b\n\n(%s)\n", " binary", sizeof( ibd ), ibd, sizeof( ibd ), "int array" );
	errno = ECONNABORTED;
	result = LogErrSystem( log, LogLevel_Critical, "system error message" );
	errno = 0;
	result = LogErrMoar( log, LogLevel_Error, FUNC_RESULT_FAILED_MEM_ALLOCATION, "moar error message" );
	result = LogClose( &log );

	return result;
}

int initCliArgs(MoardCliArgs_T* cliArgs){
	if(NULL == cliArgs)
		return FUNC_RESULT_FAILED_ARGUMENT;
	cliArgs->ConfigFile = mStrDup(CONFIG_FILE);
	return FUNC_RESULT_SUCCESS;
}

int parseCliArgs(MoardCliArgs_T* cliArgs, int argc, char** argv){
	if(NULL == cliArgs)
		return FUNC_RESULT_FAILED_ARGUMENT;
	int optchar;
	int optindex;
	struct option longArgs[] = {
			{"help", no_argument, 0,'h'},
			{"config", required_argument, 0,'c'},
			{0,0,0,0}
	};
	while((optchar = getopt_long(argc, argv, "hc:", longArgs, &optindex)) != -1){
		switch(optchar){
			case 'c':
				cliArgs->ConfigFile = optarg;
				break;
			case 'h':
				printf("Nobody helps you\n");
				break;
			case '?':
				printf("Unknown argument\n");
				break;
			default:
				return FUNC_RESULT_FAILED;
		}
	}

	return FUNC_RESULT_SUCCESS;
}

char* concatPath(const char* dir, const char* file){
	size_t pathLen = strlen(dir);
	size_t dirLen = strlen(file);
	size_t fullPathLen = pathLen+dirLen+2;
	char* enabledPath = malloc(fullPathLen);
	char* separator = PATH_SEPARATOR;
	strncpy(enabledPath, dir, pathLen);
	strncpy(enabledPath+pathLen, separator, strlen(separator));
	strncpy(enabledPath+pathLen+1, file, dirLen+1);

	return enabledPath;
}

char* getLayersEnabledPath(moardSettings* settings, char* configFile){
	//make directory for enabled layers
	char* confPath = dirname(configFile);
	return concatPath(confPath, settings->LayersEnabledDir);
}

int runAllLayers(hashTable_T *moardConfig, const char *layersConfDirName) {
	if(NULL == moardConfig || NULL == layersConfDirName)
		return FUNC_RESULT_FAILED_ARGUMENT;
	DIR * d;
	struct dirent *dir;
	d = opendir(layersConfDirName);
	if(d){
		while((dir = readdir(d))!= NULL){
			if(strstr(dir->d_name,".conf") != NULL) {
				char* fullName = concatPath(layersConfDirName, dir->d_name);
				hashTable_T *layerConfig = malloc(sizeof(hashTable_T));

				MoarLibrary_T library = {0};
				int res = loadLayer(&library, fullName, moardConfig, layerConfig);
				if (FUNC_RESULT_SUCCESS == res) {

					if((layersCount[library.Info.LayerType] == 0 && library.Info.LayerType != MoarLayer_Interface) ||
							(library.Info.LayerType == MoarLayer_Interface)) {

						printf("%s by %s loaded, %d\n", library.Info.LibraryName, library.Info.Author,
							   library.Info.TargetMoarApiVersion);
						// start layer
						res = runLayer(&library, layerConfig);
						if (FUNC_RESULT_SUCCESS == res) {
							layersCount[library.Info.LayerType]++;
							queueEnqueue(&layersRunning, &library);
						}
					}
					else {
						printf("%s by %s ignored, layer of same type was already loaded\n", library.Info.LibraryName,
							   library.Info.Author);
						closeLibrary(&library);
					}
				}
				else
					printf("Loading layer from %s failed\n", fullName);
				free(fullName);
			}
		}
		closedir(d);
	}
	return FUNC_RESULT_SUCCESS;
}

int stopAllLayers() {
	while(layersRunning.Count != 0){
		MoarLibrary_T lib;
		int res = queueDequeue(&layersRunning, &lib);
		if(FUNC_RESULT_SUCCESS == res){
			int res = exitThread(&lib);
       if(FUNC_RESULT_SUCCESS == res)
            printf("thread %s exited\n",lib.Info.LibraryName);
        else
            printf("failed thread exit for %s\n",lib.Info.LibraryName);
		}
		res = closeLibrary(&lib);
        //check for empty lib closing
        if (FUNC_RESULT_SUCCESS == res)
            printf("library %s closed\n",lib.Filename);
        else
            printf("close %s failed\n",lib.Filename);
	}
}
bool validateLayersCount(){
	for(int i= MoarLayer_Interface; i<MoarLayer_LayersCount; i++){
		if(layersCount[i]<=0)
			return false;
	}
}

int main(int argc, char** argv)
{
	//TODO add log error output
	
	MoardCliArgs_T cliArgs = {0};
	int res = initCliArgs(&cliArgs);
	CHECK_RESULT(res);
	res = parseCliArgs(&cliArgs, argc, argv);
	CHECK_RESULT(res);
	

	moardSettings settings = {0};
	ifaceSocket ifaceSock = {0};
	serviceSocket servSock = {0};

	hashTable_T config = {0};
	int settingsRes = settingsLoad(&settings, cliArgs.ConfigFile, &config);
	CHECK_RESULT(settingsRes);
	int isock = bindingBindStructFunc(&config, makeIfaceSockBinding, &ifaceSock);
	CHECK_RESULT(isock);
	int ssock = bindingBindStructFunc(&config, makeServSockBinding, &servSock);
	CHECK_RESULT(ssock);

    //setup signal handler
    signal(SIGINT, signalHandler);
	// create all needed sockets
	SocketsPrepare( ifaceSock.FileName, servSock.FileName );

	queueInit(&layersRunning, sizeof(MoarLibrary_T));

	char* layersConfDirName = getLayersEnabledPath(&settings, cliArgs.ConfigFile);

	runAllLayers(&config, layersConfDirName);

	free(layersConfDirName);

	int returnCode = 0;
	if(validateLayersCount())
    	pause();
	else {
		printf("Not all needed layers was loaded\n");
		returnCode = 1;
	}

	stopAllLayers();

	queueDeinit(&layersRunning);
    return returnCode;
}