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
#include <getopt.h>
#include <moarCommonSettings.h>
#include <libgen.h>
#include <dirent.h>
#include <moarLibrary.h>
#include <queue.h>

#define PATH_SEPARATOR 				"/"

#ifndef LOAD_MULTIPLE_INTERFACES
#define LAYERS_COUNT	(MoarLayer_LayersCount)
#else
#define LAYERS_COUNT	(MoarLayer_LayersCount+1)
#endif

typedef struct{
	char* ConfigFile;
} MoardCliArgs_T;

Queue_T layersRunning = {0};

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
	cliArgs->ConfigFile = CONFIG_FILE;
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
	size_t pathSize = strlen(dir);
	size_t dirLen = strlen(file);
	size_t fullPath = pathSize+dirLen+2;
	char* enabledPath = malloc(fullPath);
	char* separator = PATH_SEPARATOR;
	strncpy(enabledPath, dir, pathSize);
	strncpy(enabledPath+pathSize, separator, 1);
	strncpy(enabledPath+pathSize+1, file, dirLen+1);

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
				int res = loadLayer(&library, fullName, &config, layerConfig);
				int res = loadLayer(&library, fullName, moardConfig, layerConfig);

				if (FUNC_RESULT_SUCCESS == res) {
					printf("%s by %s loaded, %d\n", library.Info.LibraryName, library.Info.Author,
						   library.Info.TargetMoarApiVersion);
					// start layer
					runLayer(&library, layerConfig);
					res = runLayer(&library, layerConfig);
					if(FUNC_RESULT_SUCCESS == res)
						queueEnqueue(&layersRunning, &library);
				}
				else
					printf("%s load failed\n", fullName);

				free(fullName);
			}
		}
		closedir(d);
	}
	free(layersConfDirName);
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
}int main(int argc, char** argv)
{
	//TODO add log error output
	// TODO use get opts

	MoardCliArgs_T cliArgs = {0};
	int res = initCliArgs(&cliArgs);
	CHECK_RESULT(res);
	res = parseCliArgs(&cliArgs, argc, argv);
	CHECK_RESULT(res);
	
	moardSettings settings = {0};

	ifaceSocket ifaceSock = {0};
	serviceSocket servSock = {0};

	//LogWorkIllustration();
    //setup signal handler

	queueInit(&layersRunning, sizeof(MoarLibrary_T));

	char* layersConfDirName = getLayersEnabledPath(&settings, configFile);

	runAllLayers(&config, layersConfDirName);

	free(layersConfDirName);

    pause();


	stopAllLayers();

	queueDeinit(&layersRunning);
    return 0;
}