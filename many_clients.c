#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/param.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "hash.h"

#define CONFIG_FILENAME	"config.txt"
#define SOCK_FLNM_SZ	108 // limited with length of [struct sockadddr_un].sun_path
#define MAX_CLIENTS		10
#define BUF_SIZE		32
#define POWER_BUF_SIZE	10
#define MSG_SIZE		256
#define ERRMSG_SIZE		256
#define ADDR_SIZE		30
#define POWER_CONSTANT	63
#define LIGHT_SPEED		299792458
#define TIME_SIZE		30
#define M_PI			3.14159265358979323846

typedef struct sockaddr_un	SockAddr_T;
typedef int					Addr_T;

typedef struct {
	bool	isPresent;
	int		sock;
	float	x, y, sens;
} AddrData_T;

typedef struct {
	AddrData_T	addr_data[ HASH_CONSTANT ];
	int			sock_hash[ HASH_CONSTANT ],
				sock_to_addr[ HASH_CONSTANT ],
				addr_hash[ HASH_CONSTANT ],
				pollfd, sock; // sock is already exist in struct AddrData_T
	SockAddr_T	stSockAddr;
	float		coefficient; // after readConfig() or serverInit() will be equal to (4 * Pi * frequency (in MHz) / LIGHT_SPEED)
	char		socketFilename[ SOCK_FLNM_SZ ];
} Config_T;

const char * getTime( void ) {
	static struct timeval	moment;
	static size_t		len;
	static char		buf[ TIME_SIZE ];

	gettimeofday( &moment, NULL );
	strftime( buf, TIME_SIZE, "%F %T", localtime( &( moment.tv_sec ) ) );
	len = strlen( buf );
	snprintf( buf + len, TIME_SIZE - len - 1, ".%03d", ( int )moment.tv_usec / 1000 );
	return buf;
}

void printTimely( FILE * stream , const char * format, ... ) {
	va_list	args;

	fprintf( stream, "%s : ", getTime() );
	va_start( args, format );
	vfprintf( stream, format, args );
	va_end( args );
	}

static inline void socketKill( int sock ) {
	shutdown( sock, SHUT_RDWR );
	close( sock );
}

static inline const int getSocket( const Addr_T addr, const Config_T * cfg ) {
	return cfg->addr_data[ Search_Hash( cfg->addr_hash, addr ) ].sock;
}

static inline const Addr_T getAddr( const int sock, const Config_T * cfg ) {
	return cfg->sock_to_addr[ Search_Hash( cfg->sock_hash, sock ) ];
}

static inline AddrData_T * getData( const Addr_T addr, Config_T * cfg ) {
	return cfg->addr_data + Search_Hash( cfg->addr_hash, addr );
}

int die( int sock, const char *str ) {
	printTimely( stderr, str );
	socketKill( sock );
	exit( EXIT_FAILURE );
}

void readConfig( Config_T * cfg ) {
	int			addr, clientsLimit;
	float		x, y, sens;
	FILE		* configFile;
	AddrData_T	* curData;

	configFile = fopen( CONFIG_FILENAME, "r" );
	fscanf( configFile, "%s%f%d", cfg->socketFilename, &( cfg->coefficient ), &clientsLimit ); // here coefficient is equal to frequency
	cfg->coefficient = 4.0 * M_PI * LIGHT_SPEED / cfg->coefficient;
	Init_Hash( cfg->addr_hash );

	for( int i = 0; i < clientsLimit; i++ ) {
		fscanf( configFile, "%d%f%f%f", &addr, &x, &y, &sens );
		curData = cfg->addr_data + Add_Hash( cfg->addr_hash, addr );
		curData->x = x;
		curData->y = y;
		curData->sens = sens;
		curData->isPresent = false;
	}

	fclose( configFile );
}

void serverInit( Config_T * cfg ){
	struct epoll_event	ev;

	cfg->sock = socket( AF_UNIX, SOCK_STREAM, 0 );

	if( -1 == cfg->sock )
		die( 0, "ERROR: socket() failed" );

	memset( &( cfg->stSockAddr ), 0, sizeof( SockAddr_T ) );
	cfg->stSockAddr.sun_family = AF_UNIX;
	getcwd( cfg->stSockAddr.sun_path, SOCK_FLNM_SZ );
	strncat( cfg->stSockAddr.sun_path, "/" , SOCK_FLNM_SZ );
	strncat( cfg->stSockAddr.sun_path, cfg->socketFilename , SOCK_FLNM_SZ );
	unlink( cfg->stSockAddr.sun_path );

	if( -1 == bind( cfg->sock, ( struct sockaddr * )&( cfg->stSockAddr ), sizeof( SockAddr_T ) ) )
		die( cfg->sock, "ERROR: bind() failed" );

	if( -1 == listen( cfg->sock, 1 ) )
		die( cfg->sock, "ERROR: listen() failed" );

	cfg->pollfd = epoll_create( MAX_CLIENTS );

	if( -1 == cfg->pollfd )
		die( cfg->sock, "ERROR: epoll_create() failed" );

	ev.events = EPOLLIN;
	ev.data.fd = cfg->sock;

	if( -1 == epoll_ctl( cfg->pollfd, EPOLL_CTL_ADD, cfg->sock, &ev ) )
		die( cfg->sock, "ERROR: Init::epoll_ctl() failed" );
}

float distance( float x1, float y1, float x2, float y2 ) {
	register float	dx = x2 - x1,
					dy = y2 - y1;
	return dx * dx + dy * dy;
}

float leftPower( AddrData_T * to, const AddrData_T * from, float startPower, float coefficient ) {
	float finishPower = startPower - 20.0 * log10f( coefficient * distance( to->x, to->y, from->x, from->y ) );
	return finishPower < startPower ? finishPower : startPower;
}

static inline char * unpack( char buf[], float * power, int * size ) {
	char	* end;

	*power = strtof( buf, &end );
	*size = ( int )strtol( end, &end, 10 );
	return end + strspn( end, "\t\n\r " );
}

int transmitData( Config_T * cfg, int clientfd, char buf[], int bytesRead, Addr_T senderAddr, AddrData_T * senderData ) {
	int			bytesExpected,
				bytesShift,
				bytesFirst,
				bytesMissed;
	float		startPower,
				finishPower;
	char		powerBuf[ POWER_BUF_SIZE ],
				* textStart,
				* message;
	AddrData_T	* receiverData;

	textStart = unpack( buf + 1, &startPower, &bytesExpected );
	bytesMissed = textStart - buf;
	bytesRead -= bytesMissed;
	bytesFirst = ( bytesRead < bytesExpected ? bytesRead : bytesExpected );
	message = ( char * )calloc( bytesExpected, sizeof( char ) );
	memcpy( message, textStart, bytesFirst );
	printTimely( stdout, "Node %08X : - message : %6.3f %d %.*s", senderAddr, startPower, bytesExpected, bytesFirst, textStart );

	if( textStart[ bytesFirst - 1 ] != '\n' )
		fprintf( stdout, "\n" );

	while( bytesExpected > bytesRead ) {
		bytesShift = read( clientfd, message + bytesRead, bytesExpected - bytesRead );

		if( 0 < bytesShift )
			bytesRead += bytesShift;
		else
			break;
	}

	for( int receiverIndex = 0; receiverIndex < HASH_CONSTANT; receiverIndex++ ) {
		receiverData = cfg->addr_data + receiverIndex;

		if( cfg->addr_hash[ receiverIndex ] == HASH_EMPTY || cfg->addr_hash[ receiverIndex ] == HASH_DELETED ||
			receiverData->sock == clientfd || !( receiverData->isPresent ) )
			continue;

		finishPower = leftPower( receiverData, senderData, startPower, cfg->coefficient );

		if( finishPower > receiverData->sens ) {
			snprintf( powerBuf, sizeof( powerBuf ), "%6.3f %n", finishPower, &bytesShift );
			write( receiverData->sock, powerBuf, bytesShift );
			write( receiverData->sock, message, bytesRead );
			write( receiverData->sock, "\n", 1 );
			printTimely( stdout, "Node %08X : + message from node %08X\n", getAddr( receiverData->sock, cfg ), senderAddr );
		}
	}

	free( message );
	return bytesExpected + bytesMissed;
}

static inline int getFloat( char * buffer, float * value ) {
	static char	* end;

	if( NULL == buffer )
		return 0;

	if( NULL == value )
		strtof( buffer, &end );
	else
		*value = strtof( buffer, &end );

	return end - buffer;
}

static inline int action( char * source, float * variable, Addr_T addr, const char message[] ) {
	static int	result;

	result = getFloat( source + 1, variable );
	printTimely( stdout, "Node %08X : %s : %f\n", addr, message, *variable );
	return result + 1;
}

void processData( Config_T * cfg, int clientfd ) {
	static int			bytesDone,
						bytesShift,
						bytesLeft;
	static char			buf[ BUF_SIZE ],
						* start;
	static Addr_T		senderAddr;
	static AddrData_T	* senderData;

	memset( buf, 0, sizeof( buf ) );
	bytesLeft = read( clientfd, buf, sizeof( buf ) );

	if( bytesLeft <= 0 )
		return;

	senderAddr = getAddr( clientfd, cfg );
	senderData = getData( senderAddr, cfg );
	bytesDone = 0;
	start = buf;

	while( 0 < bytesLeft ) {
		switch( *start ) {
			case '$' : // sensitivity change
				bytesShift = action( start, &( senderData->sens ), senderAddr, "new sensitivity" );
				break;

			case 'x' : // x position change
				bytesShift = action( start, &( senderData->x ), senderAddr, "new X position" );
				break;

			case 'y' : // y position change
				bytesShift = action( start, &( senderData->y ), senderAddr, "new Y position" );
				break;

			case ':' : // data transmission
				bytesShift = transmitData( cfg, clientfd, start, bytesLeft, senderAddr, senderData );
				break;

			default :
				bytesShift = 1;
		}
		bytesDone += bytesShift;
		bytesLeft -= bytesShift;
		start += bytesShift;
	}
}

static inline int socketUnblock( int sock ) {
	int	oldFlags;

	if( -1 == sock || 0 == sock )
		printTimely( stderr, "ERROR: wrong file descriptor\n" );
	else if( -1 == ( oldFlags = fcntl( sock, F_GETFL ) ) )
		printTimely( stderr, "ERROR: fcntl( F_GETFL )\n" );
	else if( -1 == fcntl( sock, F_SETFL, oldFlags | O_NONBLOCK ) )
		printTimely( stderr, "ERROR: fcntl( F_SETFL )\n" );
	else
		return 0;

	return -1;
}

void clientRegister( Config_T * cfg ) {
	static struct 	epoll_event ev;
	int 			pos;
	int 			newsock = accept( cfg->sock, 0, 0 );
	static char 	buf[ ADDR_SIZE ];
	int 			addr, index;

	if( -1 == newsock )
		die( cfg->sock, "ERROR: accept() failed" );

	printTimely( stdout, "Socket %4d : opened\n", newsock );

	//receiving of node's address
	while( 1 ) {
		memset( buf, 0, sizeof( buf ) );
		if( 0 == read( newsock, ( void * ) buf, sizeof( buf ) ) ) {
			socketKill( newsock );
			return;
		}
		addr = strtol( buf, NULL, 0 );
		pos = Search_Hash( cfg->addr_hash, addr );
		if( -1 < pos && !( cfg->addr_data[ pos ].isPresent ) ) {
			if( -1 == socketUnblock( newsock ) ) {
				printTimely( stderr, "ERROR: socket unblocking failed\n" );
				socketKill( newsock );
			} else {
				cfg->addr_data[ pos ].isPresent = true;
				cfg->addr_data[ pos ].sock = newsock;
				index = Add_Hash( cfg->sock_hash, newsock );
				cfg->sock_to_addr[ index ] = addr;
				printTimely( stdout, "Socket %4d : node %08X registered\n", newsock, addr );
			}
			break;
		} else {
			if( -1 == pos ){
				printTimely( stdout, "Socket %4d : invalid address %d\n", newsock, addr);
				write( newsock, "Invalid address\n", 17 );
			} else {
				printTimely( stdout, "Socket %4d : busy address %d\n", newsock , addr);
				write( newsock, "Busy address\n", 14 );
			}
		}
	}

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = newsock;

	if( -1 == epoll_ctl( cfg->pollfd, EPOLL_CTL_ADD, newsock, &ev ) )
		die( cfg->sock, "ERROR: ClientRegister::epoll_ctl() failed" );
}

void clientUnregister( Config_T * cfg, int clientfd ) {
	static struct	epoll_event ev;
	int				index, addr;

	if( -1 == epoll_ctl( cfg->pollfd, EPOLL_CTL_DEL, clientfd, &ev) )
		die( cfg->sock, "ERROR: ClientUnregister::epoll_ctl() failed" );

	index = Delete_Hash( cfg->sock_hash, clientfd );
	addr = cfg->sock_to_addr[ index ];
	index = Search_Hash( cfg->addr_hash, addr );
	cfg->addr_data[ index ].isPresent = false;
	printTimely( stdout, "Socket %4d : node %08X unregistered\n", clientfd, addr );
	socketKill( clientfd );
	printTimely( stdout, "Socket %4d : closed\n", clientfd );
}

void serverWork( Config_T * cfg ) {
	struct epoll_event	events[ MAX_CLIENTS ] = {{ 0 }};
	int					eventsCount, eventIndex;;
	bool				doNotStop = true;

	Init_Hash( cfg->sock_hash );

	while( doNotStop ) {
		eventsCount = epoll_wait( cfg->pollfd, events, MAX_CLIENTS, -1 );

		if( -1 == eventsCount )
			die( cfg->sock, "ERROR: epoll_wait() failed" );

		for( eventIndex = 0; eventIndex < eventsCount; eventIndex++ )
			if( events[ eventIndex ].data.fd == cfg->sock )
				clientRegister( cfg );
			else {
				if( events[ eventIndex ].events & EPOLLIN )
					processData( cfg, events[ eventIndex ].data.fd );
				if( events[ eventIndex ].events & ( EPOLLHUP | EPOLLERR ) )
					clientUnregister( cfg, events[ eventIndex ].data.fd );
			}
	}
}

void serverStop( Config_T * cfg ) {
	socketKill( cfg->sock );
	unlink( cfg->stSockAddr.sun_path );
	socketKill( cfg->pollfd );
}

int main( void ) {
	Config_T	configStruct;

	readConfig( &configStruct );
	serverInit( &configStruct );
	serverWork( &configStruct );
	serverStop( &configStruct );

	return 0;
}
