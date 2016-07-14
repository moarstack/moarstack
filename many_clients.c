#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/param.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "hash.h"

#define printTimely( file , ...){ \
        fprintf ( file, "%s : ", getTime() );\
        fprintf ( file, __VA_ARGS__ );\
}

#define CONFIG_FILENAME	"config.txt"
#define SOCK_FLNM_SZ	108 // limited with length of [struct sockadddr_un].sun_path
#define MAX_CLIENTS	10
#define BUF_SIZE	256
#define MSG_SIZE	256
#define ERRMSG_SIZE	256
#define ADDR_SIZE	30
#define POWER_CONSTANT	63
#define LIGHT_SPEED	299792458
#define TIME_SIZE	30
#define M_PI		3.14159265358979323846

typedef struct {
	bool isPresent;
	int sock;
	float x, y, sens;
} AddrData_T;

typedef struct sockaddr_un SockAddr_T;

typedef struct {
	AddrData_T	addr_data[ HASH_CONSTANT ];
	int		sock_hash[ HASH_CONSTANT ],
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
	snprintf( buf + len, TIME_SIZE - len - 1, ".%03d", moment.tv_usec / 1000 );
	return buf;
}

void Close_socket(int sock){
	shutdown(sock, SHUT_RDWR);
        close(sock);
}

int Die(int sock, const char *str) {
	printTimely( stderr,  str);
	//close(sock);
	Close_socket(sock);
	exit(EXIT_FAILURE);
}

void readConfig( Config_T * cfg ) {
//void readConfig( int addr_hash[], AddrData_T addr_data[], float * coefficient, char * socketFilename ) {
	int	index, addr, clientsLimit;
	float	x, y, sens;
	FILE	* configFile;

	configFile = fopen( CONFIG_FILENAME, "r" );
	fscanf( configFile, "%s%f%d", cfg->socketFilename, &( cfg->coefficient ), &clientsLimit ); // here coefficient is equal to frequency
	cfg->coefficient = 4.0 * M_PI * LIGHT_SPEED / cfg->coefficient;
	Init_Hash( cfg->addr_hash );

	for( int i = 0; i < clientsLimit; i++ ) {
		fscanf( configFile, "%d%f%f%f", &addr, &x, &y, &sens );
		index = Add_Hash( cfg->addr_hash, addr );
		cfg->addr_data[ index ].x = x;
		cfg->addr_data[ index ].y = y;
		cfg->addr_data[ index ].sens = sens;
		cfg->addr_data[ index ].isPresent = false;
	}

	fclose( configFile );
}

void serverInit(Config_T * cfg){
//void serverInit( int *sock, SockAddr_T * stSockAddr, int *pollfd, const char * socketFilename ) {
	struct epoll_event	ev;

	cfg->sock = socket( AF_UNIX, SOCK_STREAM, 0 );

	if( -1 == cfg->sock)
		Die( 0, "ERROR: socket() failed" );

	memset( &(cfg->stSockAddr), 0, sizeof( SockAddr_T ) );
	cfg->stSockAddr.sun_family = AF_UNIX;
	getcwd( cfg->stSockAddr.sun_path, SOCK_FLNM_SZ );
	strncat( cfg->stSockAddr.sun_path, "/" , SOCK_FLNM_SZ );
	strncat( cfg->stSockAddr.sun_path, cfg->socketFilename , SOCK_FLNM_SZ );
	unlink( cfg->stSockAddr.sun_path );

	if( -1 == bind( cfg->sock, ( struct sockaddr * )&(cfg->stSockAddr), sizeof(SockAddr_T ) ) )
		Die( cfg->sock, "ERROR: bind() failed" );

	if( -1 == listen( cfg->sock, 1 ) )
		Die( cfg->sock, "ERROR: listen() failed" );

	cfg->pollfd = epoll_create( MAX_CLIENTS );

	if( -1 == cfg->pollfd )
		Die( cfg->sock, "ERROR: epoll_create() failed" );

	ev.events = EPOLLIN;
	ev.data.fd = cfg->sock;

	if( -1 == epoll_ctl( cfg->pollfd, EPOLL_CTL_ADD, cfg->sock, &ev) )
		Die( cfg->sock, "ERROR: Init::epoll_ctl() failed" );
}

float distance( float x1, float y1, float x2, float y2 ) {
	register float dx = x2 - x1, dy = y2 - y1;
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

void transmitData(Config_T * cfg, int clientfd){
//void transmitData(int clientfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], int sock_to_addr[], float coefficient ) {
	int 		pos, bytes;
	static char 	buf[BUF_SIZE], msg[MSG_SIZE];
	static float 	power;

	memset(buf, 0, sizeof(buf));
	bytes = write(clientfd, buf, sizeof(buf));
	if (bytes <= 0)
		return;

	if (buf[bytes - 1] == '\n')
		printTimely( stdout, "Server receive message from socket %d : %s", clientfd, buf)
	else
		printTimely( stdout, "Server receive message from socket %d : %s\n",clientfd, buf);
	
	// search position of socket in sock_hash
	pos = Search_Hash(cfg->sock_hash, clientfd);
	// search position of address of socket in addr_hash
	pos = Search_Hash(cfg->addr_hash, cfg->sock_to_addr[pos]);

	if (buf[0] == '&') {
		float sens = strtof(buf + 1, 0);
		cfg->addr_data[pos].sens = sens;
		printTimely( stdout, "Client's sensibility has been changed to %f\n", sens);
	} else {
		float power2;
		char *p;
		Unpacking(buf, &p, &power);
		bytes -= p - buf;
		//while (bytes > 0) {
			for( int receiver = 0; receiver < HASH_CONSTANT; receiver++ ) {
				if( receiver == pos || !cfg->addr_data[ receiver ].isPresent )
					continue;

				power2 = leftPower( cfg->addr_data + receiver, cfg->addr_data + pos, power, cfg->coefficient );

				if( power2 > cfg->addr_data[ receiver ].sens ) {
					sprintf( msg, "%f %s", power2, p );
					read( cfg->addr_data[ receiver ].sock, msg, sizeof(msg));
					printTimely( stdout, "Client %d send message to client %d",cfg->sock_to_addr[Search_Hash(cfg->sock_hash, clientfd)], cfg->addr_hash[ receiver ]);
				}
			}
			bytes = write(clientfd, buf, sizeof(buf));
			p = buf;
		//}
	}
}

void ClientRegister(Config_T * cfg){ 
//void ClientRegister(int sock, int pollfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], int sock_to_addr[]) {
	static struct 	epoll_event ev;
	int 		pos;
	int 		newsock = accept(cfg->sock, 0,0);
	if (newsock == -1)
		Die(cfg->sock, "ERROR: accept() failed");

	printTimely( stdout, "Socket %d is added\n", newsock);

	//receiving of node's address
	static char buf[ADDR_SIZE], msg[ERRMSG_SIZE];
	int addr, index;
	while (1){
		memset(buf, 0, sizeof(buf));
		if( 0 == read(newsock, (void *) buf, sizeof(buf)) ) {
			//shutdown(newsock, SHUT_RDWR);
			//close(newsock);
			Close_socket(newsock);
			return;
		}
		addr = atoi(buf);
		pos = Search_Hash(cfg->addr_hash, addr);
		if (pos > -1 && !cfg->addr_data[pos].isPresent) {
			cfg->addr_data[pos].isPresent = true;
			cfg->addr_data[pos].sock = newsock;
			index = Add_Hash(cfg->sock_hash, newsock);
			cfg->sock_to_addr[index] = addr;
			printTimely( stdout, "Client %d has been registered\n", addr);
			break;
		} else {
			if (pos == -1){
				printTimely( stdout, "%d is invalid address\n", addr);
				//sprintf(msg, "you have invalid address\n");
				read(newsock,  "you have invalid address\n", 25 );
			} else {
				printTimely( stdout, "%d has already registered\n", addr);
				//sprintf(msg, "this address was registered\n");
				read(newsock, "this address was registered\n", 28 );
			}
			//read(newsock, msg, strlen(msg));
		}
	}

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = newsock;
	if (epoll_ctl(cfg->pollfd, EPOLL_CTL_ADD, newsock, &ev) == -1)
		Die(cfg->sock, "ERROR: ClientRegister::epoll_ctl() failed");
}

void ClientUnregister(Config_T * cfg, int clientfd){
//void ClientUnregister(int sock, int pollfd, int clientfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], int sock_to_addr[]) {
	static struct epoll_event ev;
	if (epoll_ctl(cfg->pollfd, EPOLL_CTL_DEL, clientfd, &ev) == -1)
		Die(cfg->sock, "ERROR: ClientUnregister::epoll_ctl() failed");

	int index = Delete_Hash(cfg->sock_hash, clientfd);
	int addr = cfg->sock_to_addr[index];

	index = Search_Hash(cfg->addr_hash, addr);
	cfg->addr_data[index].isPresent = false;

	printTimely( stdout, "Client %d has been unregistered\n", addr);

	//shutdown(clientfd, SHUT_RDWR);
	//close(clientfd);
	Close_socket(clientfd);
}

void serverWork(Config_T * cfg){
//void serverWork(int sock, int pollfd, int addr_hash[], AddrData_T addr_data[], float coefficient ) {
	struct epoll_event	events[ MAX_CLIENTS ] = { 0 };
	int			eventsCount, eventIndex;
	bool			doNotStop = true;

	Init_Hash( cfg->sock_hash );

	while( doNotStop ) {
		eventsCount = epoll_wait(cfg->pollfd, events, MAX_CLIENTS, -1 );

		if( -1 == eventsCount )
			Die( cfg->sock, "ERROR: epoll_wait() failed" );

		for( eventIndex = 0; eventIndex < eventsCount; eventIndex++ )
			if( events[ eventIndex ].data.fd == cfg->sock )
				ClientRegister(cfg);
			else {
				if ( events[ eventIndex ].events & EPOLLIN)
					transmitData(cfg, events[ eventIndex ].data.fd);
				if ( events[ eventIndex ].events & ( EPOLLHUP | EPOLLERR ) )
					ClientUnregister( cfg, events[ eventIndex ].data.fd);
			}
	}
}

void Deinit(Config_T * cfg){
//void Deinit(int sock, struct sockaddr_un stSockAddr, int pollfd) {
	//shutdown(sock, SHUT_RDWR);
	//close(sock);
	Close_socket(cfg->sock);
	unlink(cfg->stSockAddr.sun_path);
	Close_socket(cfg->pollfd);
	//close(pollfd);
}

int main(void) {
	Config_T	configStruct;
	
	readConfig( &configStruct );
	serverInit( &configStruct );
	serverWork( &configStruct );
	Deinit( &configStruct );

	return 0;
}
