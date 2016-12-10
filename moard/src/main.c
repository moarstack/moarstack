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

#define PATH_SEPARATOR 				"/"
#define IFACE_CHANNEL_SOCKET_FILE	"/tmp/moarChannel.sock"
#define IFACE_LOG_FILE				"/tmp/moarInterface.log"
#define SERVICE_APP_SOCKET_FILE		"/tmp/moarService.sock"
//#define LOAD_MULTIPLE_INTERFACES

#ifndef LOAD_MULTIPLE_INTERFACES
#define LAYERS_COUNT	(MoarLayer_LayersCount)
#else
#define LAYERS_COUNT	(MoarLayer_LayersCount+1)
#endif

typedef struct{
	char* ConfigFile;
} MoardCliArgs_T;

void signalHandler(int signo){
    if(SIGINT == signo){
        printf("Received SIGINT, terminating\n");
    }
    else
        printf("Received signal %d\n",signo);
}

int runLayer( MoarLibrary_T * layerLibrary ) {
	int				result;
    void			* vsp;
    MoarLayerType_T	layerType = layerLibrary->Info.LayerType;

    if( MoarLayer_Interface == layerType ) {  // I DONT LIKE THIS PLACE
		MoarIfaceStartupParams_T	* spIface;

		spIface = ( MoarIfaceStartupParams_T * )calloc( 1, sizeof( MoarIfaceStartupParams_T ) );

		if( NULL == spIface )
			return -1;

		strncpy( spIface->socketToChannel, IFACE_CHANNEL_SOCKET_FILE, SOCKET_FILEPATH_SIZE ); // I DONT LIKE THIS PLACE
		strncpy( spIface->filepathToLog, IFACE_LOG_FILE, LOG_FILEPATH_SIZE ); // I DONT LIKE THIS PLACE TOO
		vsp = spIface;
    } else {
        MoarLayerStartupParams_T	* spNonIface;

		spNonIface = ( MoarLayerStartupParams_T * )calloc( 1, sizeof( MoarLayerStartupParams_T ) );

        if( NULL == spNonIface )
            return -1;

		spNonIface->DownSocketHandler = SocketDown( layerType );
		spNonIface->UpSocketHandler = SocketUp( layerType );
        vsp = spNonIface;
    }

	result = createThread( layerLibrary, vsp );
	printf( FUNC_RESULT_SUCCESS == result ? "%s started\n" : "failed starting %s\n", layerLibrary->Info.LibraryName );
	return result;
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

char* concatPath(char* dir, char* file){
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
int main(int argc, char** argv)
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

	
	{
		int confPrepareRes = configInit(&config);
		if (FUNC_RESULT_SUCCESS != confPrepareRes) {
		}
	int confRes = configRead(&config, cliArgs.ConfigFile);
	if (FUNC_RESULT_SUCCESS != confRes) {
		fprintf(stderr, "Can not read core config file %s\r\n", cliArgs.ConfigFile);
		return 1;

	int settingsRes = settingsLoad(&settings, configFile, &config);
	CHECK_RESULT(settingsRes);
	int isock = bindingBindStructFunc(&config, makeIfaceSockBinding, &ifaceSock);
	CHECK_RESULT(isock);
	int ssock = bindingBindStructFunc(&config, makeServSockBinding, &servSock);
	CHECK_RESULT(ssock);
    MoarLibrary_T libraries[LAYERS_COUNT];

	LogWorkIllustration();

    char *fileNames[LAYERS_COUNT];
    fileNames[MoarLayer_Interface] = LIBRARY_PATH_INTERFACE;
    fileNames[MoarLayer_Channel] = LIBRARY_PATH_CHANNEL;
    fileNames[MoarLayer_Routing] = LIBRARY_PATH_ROUTING;
    fileNames[MoarLayer_Presentation] = LIBRARY_PATH_PRESENTATION;
    fileNames[MoarLayer_Service] = LIBRARY_PATH_SERVICE;
    //testing multiple instances
#ifdef LOAD_MULTIPLE_INTERFACES
    fileNames[MoarLayer_Service+1] = LIBRARY_PATH_INTERFACE;
#endif

    //setup signal handler
    signal(SIGINT, signalHandler);
	// create all needed sockets
	SocketsPrepare( ifaceSock.FileName, servSock.FileName );

	char* layersConfDirName = getLayersEnabledPath(&settings, configFile);
	DIR * d;
	struct dirent *dir;
	d = opendir(layersConfDirName);
	if(d){
		while((dir = readdir(d))!= NULL){
			if(strstr(dir->d_name,".conf") != NULL) {
				// file here
				hashTable_T *layerConfig = malloc(sizeof(hashTable_T));
				//init config
				int res = configInit(layerConfig);
				CHECK_RESULT(res);
				// get config
				char* fullName = concatPath(layersConfDirName, dir->d_name);
				res = configRead(layerConfig, fullName);
				free(fullName);
				CHECK_RESULT(res);
				// merge without override
				res = configMerge(layerConfig, &config);
				CHECK_RESULT(res);
				// get file name
				libraryLocation location = {0};
				res = bindingBindStructFunc(layerConfig, makeLibraryLocationBinding, &location);
				CHECK_RESULT(res);
				// load layer
				MoarLibrary_T library = {0};
				res = loadLibrary(location.FileName, &library);
				if (FUNC_RESULT_SUCCESS == res) {
					printf("%s by %s loaded, %d\n", library.Info.LibraryName, library.Info.Author,
						   library.Info.TargetMoarApiVersion);
					// start layer
					runLayer(&library);
				}
				else
					printf("%s load failed\n", location.FileName);
			}
		}
		closedir(d);
	}
	free(layersConfDirName);




    //load
    for(int i=0; i<LAYERS_COUNT;i++) {
        int res = loadLibrary(fileNames[i], libraries+i);
        if (FUNC_RESULT_SUCCESS == res)
            printf("%s by %s loaded, %d\n", libraries[i].Info.LibraryName, libraries[i].Info.Author, libraries[i].Info.TargetMoarApiVersion);
        else
            printf("%s load failed\n",fileNames[i]);
    }

    //start layers here
	for(int i = 0; i < LAYERS_COUNT; i++)
		runLayer( libraries + i );



    //wait
    pause();
    //stop
    for(int i=0; i<LAYERS_COUNT;i++) {
        int res = exitThread(libraries+i);
        if(FUNC_RESULT_SUCCESS == res)
            printf("thread %s exited\n",libraries[i].Info.LibraryName);
        else
            printf("failed thread exit for %s\n",libraries[i].Info.LibraryName);
    }
    //unload
    for(int i=0; i<LAYERS_COUNT;i++) {
        int res = closeLibrary(libraries+i);
        //check for empty lib closing
        if (FUNC_RESULT_SUCCESS == res)
            printf("library %s closed\n",libraries[i].Filename);
        else
            printf("close %s failed\n",libraries[i].Filename);
    }
    return 0;
}