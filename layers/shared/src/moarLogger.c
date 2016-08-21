//
// Created by kryvashek on 20.08.16.
//

#include <string.h>			// snprintf(), memset(), ...
#include <time.h>			// localtime(), strftime(), ...
#include <errno.h>			// errno

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

	*handle = fopen( logFile, "a" );

	return ( NULL == *handle ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

int logMoment( LogHandle_T handle ) {
	int			result;
	time_t		seconds;
	moarTime_T	moment;
	size_t		length;
	LogMoment_T	buffer = { 0 };

	moment = timeGetCurrent();
	seconds = ( time_t )( moment / 1000 );
	strftime( buffer, LOG_TIMESTAMP_SIZE, "%F %T", localtime( &seconds ) );
	length = strlen( buffer );
	result = snprintf( buffer + length, LOG_TIMESTAMP_SIZE - length - 1, ".%03u", ( unsigned )( moment % 1000 ) );

	if( 0 > result )
		return FUNC_RESULT_FAILED;

	result = fprintf( handle, "%s : ", buffer );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

// writes some message to the log file specified by handle, adding time of writing
int LogWrite( LogHandle_T handle, const char * format, ... ) {
	va_list		args;
	int			result;
	size_t		length;

	if( NULL == handle || NULL == format || 0 == strlen( format ) )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = logMoment( handle );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	va_start( args, format );
	result = vfprintf( handle, format, args );
	va_end( args );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

// writes system error message to the log file specified by handle, adding time of writing
int LogErrSystem( LogHandle_T handle, const char * message ) {
	int	result;

	if( NULL == handle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = logMoment( handle );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	if( NULL == message )
		result = fprintf( handle, "system error %d (%s)\n", errno, strerror( errno ) );
	else
		result = fprintf( handle, "system error %d (%s) : %s\n", errno, strerror( errno ), message );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

// writes moar error message to the log file specified by handle, adding time of writing
int LogErrMoar( LogHandle_T handle, int returnResult, const char * message ) {
	int	result;

	if( NULL == handle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = logMoment( handle );

	if( FUNC_RESULT_SUCCESS != result )
		return result;

	if( NULL == message )
		result = fprintf( handle, "moar error %d (%s)\n", returnResult, moarErrorMessages[ returnResult ] );
	else
		result = fprintf( handle, "moar error %d (%s) : %s\n", returnResult, moarErrorMessages[ returnResult ], message );

	return ( 0 > result ? FUNC_RESULT_FAILED_IO : FUNC_RESULT_SUCCESS );
}

// closes log file specified by given handle
int LogClose( LogHandle_T * handle ) {
	int	result;

	if( NULL == handle )
		return FUNC_RESULT_FAILED_ARGUMENT;

	result = fclose( *handle );

	if( 0 != result )
		return FUNC_RESULT_FAILED_IO;

	*handle = NULL;

	return FUNC_RESULT_SUCCESS;
}
