//
// Created by kryvashek on 20.08.16.
//

#include <string.h>			// snprintf(), memset(), ...
#include <time.h>			// localtime(), strftime(), ...
#include <errno.h>			// errno
#include <stdlib.h>			// malloc(), free()
#include <stdbool.h>		// bool

#include <moarLogger.h>
#include <moarTime.h>		// timeGetCurrent()
#include <funcResults.h>

static char	*moarErrorMessages[LOG_MOAR_ERRS_COUNT] = {
	"FUNC_RESULT_SUCCESS",
	"FUNC_RESULT_FAILED",
	"FUNC_RESULT_FAILED_ARGUMENT",
	"FUNC_RESULT_FAILED_IO",
	"FUNC_RESULT_FAILED_MEM_ALLOCATION"
};

// opens log file with specified filepath; returns handler on success, value <= 0 otherwise
int LogOpen( LogFilepath_T logFile, LogHandle_T * handle ) {
	if( NULL == logFile || NULL == handle || 0 == strlen( logFile ) )
		return FUNC_RESULT_FAILED_ARGUMENT;

	*handle = malloc( sizeof( LogDescriptor_T ) );

	if( NULL == *handle )
		return FUNC_RESULT_FAILED_MEM_ALLOCATION;

	( *handle )->FileHandle = fopen( logFile, "a" );

	if( NULL == ( *handle )->FileHandle ) {
		free( *handle );
		*handle = NULL;
		return FUNC_RESULT_FAILED_IO;
	}

	( *handle )->Delimiter = LOG_DEF_DELIMITER;
	( *handle )->MinLogLevel = LOG_DEF_LEVEL_LOG;
	( *handle )->MinDumpLevel = LOG_DEF_LEVEL_DUMP;

	return FUNC_RESULT_SUCCESS;
}

// tunes specified descriptor with given value of minimal logging level
int LogSetLevelLog( LogHandle_T handle, LogLevel_T logLevel ) {
	if( NULL == handle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	handle->MinLogLevel = logLevel;

	return FUNC_RESULT_SUCCESS;
}

// tunes specified descriptor with given value of minimal dumping level
int LogSetLevelDump( LogHandle_T handle, LogLevel_T dumpLevel ) {
	if( NULL == handle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	handle->MinDumpLevel = dumpLevel;

	return FUNC_RESULT_SUCCESS;
}

// tunes specified descriptor with given value of delimiter
int LogSetDelimiter( LogHandle_T handle, char delimiter ) {
	if( NULL == handle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	handle->Delimiter = delimiter;

	return FUNC_RESULT_SUCCESS;
}

int logMoment( LogHandle_T handle ) {
	int			result;
	time_t		seconds;
	moarTime_T	moment;
	size_t		length;

	memset( handle->MomentBuffer, 0, LOG_TIMESTAMP_SIZE );
	moment = timeGetCurrent();
	seconds = ( time_t )( moment / 1000 );
	strftime( handle->MomentBuffer, LOG_TIMESTAMP_SIZE, "%F %T", localtime( &seconds ) );
	length = strlen( handle->MomentBuffer );
	result = snprintf( handle->MomentBuffer + length, LOG_TIMESTAMP_SIZE - length - 1, ".%03u", ( unsigned )( moment % 1000 ) );

	if( 0 > result )
		return FUNC_RESULT_FAILED;

	result = fprintf( handle->FileHandle, "%s : ", handle->MomentBuffer );
	fflush( handle->FileHandle );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

char * logReformat( const char * startFormat, bool replaceDumpSpecifier ) {
	char	* finishFormat,
			* replace;
	size_t	length;

	if( NULL == startFormat )
		return NULL;

	length = strlen( startFormat );

	if( 0 == length )
		return NULL;

	finishFormat = malloc( length + 1 );

	if( NULL == finishFormat )
		return NULL;

	memcpy( finishFormat, startFormat, length );
	finishFormat[ length ] = 0;

	if( '\n' == finishFormat[ length - 1 ] )
		finishFormat[ length - 1 ] = 0;

	replace = strpbrk( finishFormat, "%\n" );

	while( NULL != replace ) {
		if( '\n' == replace[ 0 ] )
			replace[ 0 ] = ' ';
		else if( replaceDumpSpecifier && 'b' == replace[ 1 ] ) {
			replace[ 0 ] = '[';
			replace[ 1 ] = ']';
		}

		replace = strpbrk( replace + 1, "%\n" );
	}

	return finishFormat;
}

int logPrintBinary( LogHandle_T handle, va_list args ) {
	int 	result = 1;
	char	* binaryData;
	size_t	binaryDataSize;

	binaryData = va_arg( args, char* );
	binaryDataSize = va_arg( args, size_t );

	for( size_t byte = 0; byte < binaryDataSize - 1 && 0 <= result; byte++ )
		result = fprintf( handle->FileHandle, "%02X ", binaryData[ byte ] );

	if( 0 <= result )
		result = fprintf( handle->FileHandle, "%02X", binaryData[ binaryDataSize - 1 ] );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

int logPrint( LogHandle_T handle, char * format, va_list args ) {
	int 	result = 1;
	char	* next = strstr( format, "%b" );

	if( NULL == handle || NULL == format || NULL == handle->FileHandle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	while( NULL != next ) {
		next[ 0 ] = next[ 1 ] = 0;
		result = vfprintf( handle->FileHandle, format, args );

		if( 0 <= result )
			result = logPrintBinary( handle, args );
		else
			result = FUNC_RESULT_FAILED_IO;

		if( FUNC_RESULT_SUCCESS != result )
			return result;

		format = next + 2;
		next = strstr( format, "%b" );
	}

	if( 0 <= result )
		result = vfprintf( handle->FileHandle, format, args );

	if( 0 <= result )
		result = fprintf( handle->FileHandle, "\n" );

	fflush( handle->FileHandle );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

// writes some message to the log file specified by handle, adding time of writing
int LogWrite( LogHandle_T handle, LogLevel_T logLevel, const char * format, ... ) {
	va_list	args;
	int		result;
	char	* lineFormat;

	if( NULL == handle || NULL == format || NULL == handle->FileHandle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( logLevel < handle->MinLogLevel )
		return FUNC_RESULT_SUCCESS;

	lineFormat = logReformat( format, logLevel < handle->MinDumpLevel );

	if( NULL == lineFormat )
		return FUNC_RESULT_FAILED;

	result = logMoment( handle );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	va_start( args, format );
	result = logPrint( handle, lineFormat, args );
	va_end( args );
	free( lineFormat );

	return result;
}

// writes system error message to the log file specified by handle, adding time of writing
int LogErrSystem( LogHandle_T handle, LogLevel_T logLevel, const char * message ) {
	const int	errorValue = errno;
	int			result;

	if( NULL == handle || NULL == handle->FileHandle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( logLevel < handle->MinLogLevel )
		return FUNC_RESULT_SUCCESS;

	result = logMoment( handle );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	if( NULL == message )
		result = fprintf( handle->FileHandle, "system error %d (%s)\n", errorValue, strerror( errorValue ) );
	else {
		char	* lineMessage = logReformat( message, true );

		if( NULL == lineMessage )
			return FUNC_RESULT_FAILED;

		result = fprintf( handle->FileHandle, "system error %d (%s) : %s\n", errorValue, strerror( errorValue ),
						  lineMessage );
		free( lineMessage );
	}

	fprintf( handle->FileHandle, "\n" );
	fflush( handle->FileHandle );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

// writes moar error message to the log file specified by handle, adding time of writing
int LogErrMoar( LogHandle_T handle, LogLevel_T logLevel, int returnResult, const char * message ) {
	int	result;

	if( NULL == handle || NULL == handle->FileHandle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	if( logLevel < handle->MinLogLevel )
		return FUNC_RESULT_SUCCESS;

	result = logMoment( handle );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	if( NULL == message )
		result = fprintf( handle->FileHandle, "moar error %d (%s)\n", returnResult, moarErrorMessages[ returnResult ] );
	else {
		char	* lineMessage = logReformat( message, true );

		if( NULL == lineMessage )
			return FUNC_RESULT_FAILED;

		result = fprintf( handle->FileHandle, "moar error %d (%s) : %s\n", returnResult,
						  moarErrorMessages[ returnResult ], lineMessage );
		free( lineMessage );
	}

	fprintf( handle->FileHandle, "\n" );
	fflush( handle->FileHandle );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

// closes log file specified by given handle
int LogClose( LogHandle_T * handle ) {
	int	result;

	if( NULL == handle || NULL == *handle || NULL == ( *handle )->FileHandle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	fflush( ( *handle )->FileHandle );
	result = fclose( ( *handle )->FileHandle );

	if( 0 != result )
		return FUNC_RESULT_FAILED_IO;

	( *handle )->FileHandle = NULL;
	free( *handle );
	*handle = NULL;

	return FUNC_RESULT_SUCCESS;
}
