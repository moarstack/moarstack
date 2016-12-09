//
// Created by kryvashek on 20.08.16.
//

#ifndef MOARSTACK_MOARLOGGER_H
#define MOARSTACK_MOARLOGGER_H

#include <stdio.h>
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

#define LOG_CHECK_RESULT_ANY(r,h,l,f,...)	do{ LogWrite( (h), (l), (f), __VA_ARGS__ ); if( FUNC_RESULT_SUCCESS != (r) ) return (r); }while( 0 )
#define LOG_CHECK_RESULT_BAD(r,h,l,f,...)	do{ if( FUNC_RESULT_SUCCESS != (r) ) { LogWrite( (h), (l), (f), __VA_ARGS__ ); return (r); } }while( 0 )
#define LOG_CHECK_ERROR_MOAR(r,h,l,m)		do{ if( FUNC_RESULT_SUCCESS != (r) ) { LogErrMoar( (h), (l), (r), (m) ); return (r); } }while( 0 )
#define LOG_CHECK_ERROR_SYS(h,l,m)			do{ if( 0 != errno ) { LogErrSystem( (h), (l), (m) ); return FUNC_RESULT_FAILED; } }while( 0 )

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

// closes log file specified by given handle
extern int LogClose( LogHandle_T * handle );

__END_DECLS

#endif //MOARSTACK_MOARLOGGER_H
