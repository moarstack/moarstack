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
#define BUF_SIZE		256
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

const char * getTime( void ) {
	static struct timeval	moment;
	static size_t			len;
	static char				buf[ TIME_SIZE ];

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

static inline const int getSocket( const Addr_T addr, const int addr_hash[], const AddrData_T addr_data[] ) {
	return addr_data[ Search_Hash( addr_hash, addr ) ].sock;
}

static inline const Addr_T getAddr( const int sock, const int sock_hash[], const Addr_T sock_to_addr[] ) {
	return sock_to_addr[ Search_Hash( sock_hash, sock ) ];
}

static inline AddrData_T * getData( const Addr_T addr, const int addr_hash[], AddrData_T addr_data[] ) {
	return addr_data + Search_Hash( addr_hash, addr );
}

int die(int sock, const char *str) {
	printTimely( stderr, str );
	socketKill( sock );
	exit( EXIT_FAILURE );
}

void readConfig( int addr_hash[], AddrData_T addr_data[], float * coefficient, char * socketFilename ) {
	int			addr, clientsLimit;
	float		x, y, sens;
	FILE		* configFile;
	AddrData_T	* curData;

	configFile = fopen( CONFIG_FILENAME, "r" );
	fscanf( configFile, "%s%f%d", socketFilename, coefficient, &clientsLimit ); // here coefficient is equal to frequency
	*coefficient = 4.0 * M_PI * LIGHT_SPEED / *coefficient;
	Init_Hash( addr_hash );

	for( int i = 0; i < clientsLimit; i++ ) {
		fscanf( configFile, "%d%f%f%f", &addr, &x, &y, &sens );
		curData = addr_data + Add_Hash( addr_hash, addr );
		curData->x = x;
		curData->y = y;
		curData->sens = sens;
		curData->isPresent = false;
	}

	fclose( configFile );
}

void serverInit( int *sock, SockAddr_T * stSockAddr, int *pollfd, const char * socketFilename ) {
	struct epoll_event	ev;

	*sock = socket( AF_UNIX, SOCK_STREAM, 0 );

	if( -1 == *sock )
		die( 0, "ERROR: socket() failed" );

	memset( stSockAddr, 0, sizeof( SockAddr_T ) );
	stSockAddr->sun_family = AF_UNIX;
	getcwd( stSockAddr->sun_path, SOCK_FLNM_SZ );
	strncat( stSockAddr->sun_path, "/" , SOCK_FLNM_SZ );
	strncat( stSockAddr->sun_path, socketFilename , SOCK_FLNM_SZ );
	unlink( stSockAddr->sun_path );

	if( -1 == bind( *sock, ( struct sockaddr * )stSockAddr, sizeof( SockAddr_T ) ) )
		die( *sock, "ERROR: bind() failed" );

	if( -1 == listen( *sock, 1 ) )
		die( *sock, "ERROR: listen() failed" );

	*pollfd = epoll_create( MAX_CLIENTS );

	if( -1 == *pollfd )
		die( *sock, "ERROR: epoll_create() failed" );

	ev.events = EPOLLIN;
	ev.data.fd = *sock;

	if( -1 == epoll_ctl( *pollfd, EPOLL_CTL_ADD, *sock, &ev) )
		die( *sock, "ERROR: Init::epoll_ctl() failed" );
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

void Unpacking(char buf[], char **p, float* power) {
	*power = strtof(buf, p);

	while (**p == ' ')
		++(*p);
}

void transmitData(int clientfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], Addr_T sock_to_addr[], float coefficient ) {
	int 			bytes;
	static char		buf[ BUF_SIZE ], msg[ MSG_SIZE ];
	static float	power;
	Addr_T			senderAddr;
	AddrData_T		* senderData,
					* receiverData;

	memset( buf, 0, sizeof( buf ) );
	bytes = read( clientfd, buf, sizeof( buf ) );

	if( bytes <= 0 )
		return;

	senderAddr = getAddr( clientfd, sock_hash, sock_to_addr );
	senderData = getData( senderAddr, addr_hash, addr_data );

	if( '$' == buf[ 0 ] ) {
		senderData->sens = strtof( buf + 1, 0 );
		printTimely( stdout, "Node %08X : new sensitivity : %f\n", senderAddr, senderData->sens );
	} else {
		float 	power2;
		char 	*p;

		printTimely( stdout, "Node %08X : sent message : %s", senderAddr, buf );

		if( buf[ bytes - 1 ] != '\n' )
			fprintf( stdout, "\n" );

		Unpacking( buf, &p, &power );
		bytes -= p - buf;

		while( 0 < bytes ) {
			for( int receiverIndex = 0; receiverIndex < HASH_CONSTANT; receiverIndex++ ) {
				receiverData = addr_data + receiverIndex;

				if( addr_hash[ receiverIndex ] == HASH_EMPTY || addr_hash[ receiverIndex ] == HASH_DELETED ||
					receiverData->sock == clientfd || !( receiverData->isPresent ) )
					continue;

				power2 = leftPower( receiverData, senderData, power, coefficient );

				if( power2 > receiverData->sens ) {
					sprintf( msg, "%f %s", power2, p );
					write( receiverData->sock, msg, sizeof( msg ) );
					printTimely( stdout, "Node %08X : got message from node %08X\n", getAddr( receiverData->sock, sock_hash, sock_to_addr ), senderAddr );
				}
			}

			bytes = read( clientfd, buf, sizeof( buf ) );
			p = buf;
		}
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

void ClientRegister(int sock, int pollfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], Addr_T sock_to_addr[] ) {
	static struct 	epoll_event ev;
	int 		pos;
	int 		newsock = accept(sock, 0,0);
	static char 	buf[ ADDR_SIZE ];
	int 		addr, index;

	if (newsock == -1)
		die(sock, "ERROR: accept() failed");

	printTimely( stdout, "Socket %4d : opened\n", newsock );

	//receiving of node's address
	while (1){
		memset(buf, 0, sizeof(buf));
		if( 0 == read(newsock, (void *) buf, sizeof(buf)) ) {
			socketKill( newsock );
			return;
		}
		addr = atoi(buf);
		pos = Search_Hash(addr_hash, addr);
		if (pos > -1 && !addr_data[pos].isPresent) {
			addr_data[pos].isPresent = true;
			addr_data[pos].sock = newsock;
			index = Add_Hash(sock_hash, newsock);
			sock_to_addr[index] = addr;
			if( -1 == socketUnblock( newsock ) ) {
				printTimely( stderr, "ERROR: socket unblocking failed\n" );
				socketKill( newsock );
			} else
				printTimely( stdout, "Socket %4d : node %08X registered\n", newsock, addr );
			break;
		} else {
			if (pos == -1){
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
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, newsock, &ev) == -1)
		die(sock, "ERROR: ClientRegister::epoll_ctl() failed");
}

void ClientUnregister(int sock, int pollfd, int clientfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], Addr_T sock_to_addr[] ) {
	static struct 	epoll_event ev;
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, clientfd, &ev) == -1)
		die(sock, "ERROR: ClientUnregister::epoll_ctl() failed");

	int 	index = Delete_Hash(sock_hash, clientfd);
	int 	addr = sock_to_addr[index];

	index = Search_Hash(addr_hash, addr);
	addr_data[index].isPresent = false;
	printTimely( stdout, "Socket %4d : node %08X unregistered\n", clientfd, addr );
	socketKill( clientfd );
	printTimely( stdout, "Socket %4d : closed\n", clientfd );
}

void serverWork(int sock, int pollfd, int addr_hash[], AddrData_T addr_data[], float coefficient ) {
	struct epoll_event	events[ MAX_CLIENTS ] = {{ 0 }};
	int					sock_hash[ HASH_CONSTANT ] = { 0 },
						eventsCount, eventIndex;;
	Addr_T				sock_to_addr[ HASH_CONSTANT ] = { 0 };
	bool				doNotStop = true;

	Init_Hash( sock_hash );

	while( doNotStop ) {
		eventsCount = epoll_wait( pollfd, events, MAX_CLIENTS, -1 );

		if( -1 == eventsCount )
			die( sock, "ERROR: epoll_wait() failed" );

		for( eventIndex = 0; eventIndex < eventsCount; eventIndex++ )
			if( events[ eventIndex ].data.fd == sock )
				ClientRegister( sock, pollfd, addr_hash, addr_data, sock_hash, sock_to_addr );
			else {
				if ( events[ eventIndex ].events & EPOLLIN)
					transmitData( events[ eventIndex ].data.fd, addr_hash, addr_data, sock_hash, sock_to_addr, coefficient );
				if ( events[ eventIndex ].events & ( EPOLLHUP | EPOLLERR ) )
					ClientUnregister( sock, pollfd, events[ eventIndex ].data.fd, addr_hash, addr_data, sock_hash, sock_to_addr );
			}
	}
}

void Deinit( int sock, struct sockaddr_un stSockAddr, int pollfd ) {
	socketKill( sock );
	unlink( stSockAddr.sun_path );
	socketKill( pollfd );
}

int main(void) {
	AddrData_T	addr_data[ HASH_CONSTANT ] = {{ 0 }};
	int			addr_hash[ HASH_CONSTANT ] = { 0 },
				pollfd, sock;
	SockAddr_T	stSockAddr;
	float		coefficient; // after readConfig() or serverInit() will be equal to (4 * Pi * frequency (in MHz) / LIGHT_SPEED)
	char		socketFilename[ SOCK_FLNM_SZ ] = { 0 };

	readConfig( addr_hash, addr_data, &coefficient, socketFilename );
	serverInit( &sock, &stSockAddr, &pollfd, socketFilename );
	serverWork( sock, pollfd, addr_hash, addr_data, coefficient );
	Deinit( sock, stSockAddr, pollfd );

	return 0;
}
