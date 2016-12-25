//
// Created by kryvashek on 20.08.16.
//

#ifndef MOARSTACK_MOARLOGGER_H
#define MOARSTACK_MOARLOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdarg.h>

#include <funcResults.h>

#define LOG_FILEPATH_SIZE	108 // just chosen to be the same as length of SOCKET_FILEPATH_SIZE
#define LOG_TIMESTAMP_SIZE	30	// just copied from MockIT
#define LOG_MOAR_ERRS_COUNT	(FUNC_RESULT_SUCCESS-FUNC_RESULT_MINIMUM_VALUE+1)
#define LOG_BITS_FOR_LOG	4
#define LOG_BITS_FOR_DUMP	4
#define LOG_DEF_DELIMITER	'|'
#define LOG_DEF_LEVEL_LOG	LogLevel_Information
#define LOG_DEF_LEVEL_DUMP	LogLevel_Warning
#define LOG_LEVELS_COUNT	(1+(int)LogLevel_Critical)

#define LOG_CHECK_RESULT_MOAR(r,h,lb,mb,lg,mg)	do{ 																			\
													int __macros__result__ = LogCombMoar( (h), (r), (lb), (mb), (lg), (mg) );	\
													if( FUNC_RESULT_SUCCESS != __macros__result__ )								\
														return __macros__result__;												\
												} while( 0 )

#define LOG_CHECK_RESULT_SYSTEM(h,lb,mb,lg,mg)	do{																			\
													int __macros__result__ = LogCombSystem( (h), (lb), (mb), (lg), (mg) );	\
													if( FUNC_RESULT_SUCCESS != __macros__result__ )							\
														return __macros__result__;											\
												} while( 0 )

#define LOG_CHECK_ERROR_MOAR(r,h,l,m)			do{															\
													int __macros__result__ = (r);							\
													if( FUNC_RESULT_SUCCESS != __macros__result__ ) {		\
														LogErrMoar( (h), (l), __macros__result__, (m) );	\
														return __macros__result__;							\
													}														\
												} while( 0 )

#define LOG_CHECK_ERROR_SYSTEM(h,l,m)			do{										\
													if( 0 != errno ) {					\
														LogErrSystem( (h), (l), (m) );	\
														return FUNC_RESULT_FAILED;		\
													}									\
												} while( 0 )

#pragma pack(push, 1)

typedef char	LogFilepath_T[ LOG_FILEPATH_SIZE ];
typedef char	LogMoment_T[ LOG_TIMESTAMP_SIZE ];
typedef FILE	* LogFileHandle_T;

typedef enum {
	LogLevel_Dump,
	LogLevel_Debug1,
	LogLevel_DebugVerbose = LogLevel_Debug1,
	LogLevel_Debug2,
	LogLevel_Debug3,
	LogLevel_Debug4,
	LogLevel_DebugQuiet = LogLevel_Debug4,
	LogLevel_Information,
	LogLevel_Warning,
	LogLevel_Error,
	LogLevel_Critical
} LogLevel_T;

typedef struct {
	LogFileHandle_T	FileHandle;
	LogMoment_T		MomentBuffer;
	LogLevel_T		MinLogLevel:LOG_BITS_FOR_LOG,
					MinDumpLevel:LOG_BITS_FOR_DUMP;
	char			Delimiter;
} LogDescriptor_T;

typedef LogDescriptor_T	* LogHandle_T;

#pragma pack(pop)

__BEGIN_DECLS

// opens log file with specified filepath; returns handler on success, value <= 0 otherwise
extern int LogOpen( LogFilepath_T logFile, LogHandle_T * handle );

// tunes specified descriptor with given value of minimal logging level
extern int LogSetLevelLog( LogHandle_T handle, LogLevel_T logLevel );

// tunes specified descriptor with given value of minimal dumping level
extern int LogSetLevelDump( LogHandle_T handle, LogLevel_T dumpLevel );

// tunes specified descriptor with given value of delimiter
extern int LogSetDelimiter( LogHandle_T handle, char delimiter );

// writes some message to the log file specified by handle, adding time of writing
extern int LogWrite( LogHandle_T handle, LogLevel_T logLevel, const char * format, ... );

// writes system error message to the log file specified by handle, adding time of writing
extern int LogErrSystem( LogHandle_T handle, LogLevel_T logLevel, const char * message );

// writes moar error message to the log file specified by handle, adding time of writing
extern int LogErrMoar( LogHandle_T handle, LogLevel_T logLevel, int returnResult, const char * message );

// calls LogWrite with -Good arguments or LogErrMoar with -Bad ones depending on value of errno
extern int LogCombSystem( LogHandle_T handle, LogLevel_T logLevelBad, const char * msgBad, LogLevel_T logLevelGood, const char * msgGood );

// calls LogWrite with -Good arguments or LogErrMoar with -Bad ones depending on returnResult
extern int LogCombMoar( LogHandle_T handle, int returnResult, LogLevel_T logLevelBad, const char * msgBad, LogLevel_T logLevelGood, const char * msgGood );

// closes log file specified by given handle
extern int LogClose( LogHandle_T * handle );

__END_DECLS

#endif //MOARSTACK_MOARLOGGER_H
