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
#define MAX_CLIENTS		10
#define BUF_SIZE		256
#define MSG_SIZE		256
#define ERRMSG_SIZE		256
#define ADDR_SIZE		30
#define POWER_CONSTANT	63
#define LIGHT_SPEED		299792458
#define TIME_SIZE		30
#define M_PI			3.14159265358979323846

typedef struct {
	bool isPresent;
	int sock;
	float x, y, sens;
} AddrData_T;

typedef struct sockaddr_un SockAddr_T;

const char * getTime( void ) {
	static struct timeval	moment;
	static size_t			len;
	static char				buf[ TIME_SIZE ];

	gettimeofday( &moment, NULL );
	strftime( buf, TIME_SIZE, "%F %T", localtime( &( moment.tv_sec ) ) );
	len = strlen( buf );
	snprintf( buf + len, TIME_SIZE - len - 1, ".%d", moment.tv_usec / 1000 );
	return buf;
}

int Die(int sock, const char *str) {
	printTimely( stderr,  str);
	close(sock);
	exit(EXIT_FAILURE);
}

void readConfig( int addr_hash[], AddrData_T addr_data[], float * freq, char * socketFilename ) {
	int		index, addr, clientsLimit;
	float	x, y, sens;
	FILE	* configFile;

	configFile = fopen( CONFIG_FILENAME, "r" );
	fscanf( configFile, "%s%f%d", socketFilename, freq, &clientsLimit );
	Init_Hash( addr_hash );

	for( int i = 0; i < clientsLimit; i++ ) {
		fscanf( configFile, "%d%f%f%f", &addr, &x, &y, &sens );
		index = Add_Hash( addr_hash, addr );
		addr_data[ index ].x = x;
		addr_data[ index ].y = y;
		addr_data[ index ].sens = sens;
		addr_data[ index ].isPresent = false;
	}

	fclose( configFile );
}

void serverInit( int *sock, SockAddr_T * stSockAddr, int *pollfd, const char * socketFilename ) {
	struct epoll_event	ev;

	*sock = socket( AF_UNIX, SOCK_STREAM, 0 );

	if( -1 == *sock )
		Die( 0, "ERROR: socket() failed" );

	memset( stSockAddr, 0, sizeof( SockAddr_T ) );
	stSockAddr->sun_family = AF_UNIX;
	getcwd( stSockAddr->sun_path, SOCK_FLNM_SZ );
	strncat( stSockAddr->sun_path, "/" , SOCK_FLNM_SZ );
	strncat( stSockAddr->sun_path, socketFilename , SOCK_FLNM_SZ );
	unlink( stSockAddr->sun_path );

	if( -1 == bind( *sock, ( struct sockaddr * )stSockAddr, sizeof( SockAddr_T ) ) )
		Die( *sock, "ERROR: bind() failed" );

	if( -1 == listen( *sock, 1 ) )
		Die( *sock, "ERROR: listen() failed" );

	*pollfd = epoll_create( MAX_CLIENTS );

	if( -1 == *pollfd )
		Die( *sock, "ERROR: epoll_create() failed" );

	ev.events = EPOLLIN;
	ev.data.fd = *sock;

	if( -1 == epoll_ctl( *pollfd, EPOLL_CTL_ADD, *sock, &ev) )
		Die( *sock, "ERROR: Init::epoll_ctl() failed" );
}

float Distance(float x1, float y1, float x2, float y2) {
	register float dx = x2 - x1, dy = y2 - y1;
	return dx * dx + dy * dy;
}

float Power(float x1, float y1, float x2, float y2, float start_power, float freq) {
	float free_att = 20.0 * log10f(4.0 * M_PI * Distance(x1, y1, x2, y2) / (LIGHT_SPEED / freq));
	float res = start_power - free_att;
	if (start_power < res) res = start_power;
	return res;  
}

void Unpacking(char buf[], char **p, float* power) {
	*power = strtof(buf, p);
		
	while (**p == ' ')
		++(*p);
}

void TransmitData(int clientfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], int sock_to_addr[], float freq) {
	int pos, bytes;
	static char buf[BUF_SIZE], msg[MSG_SIZE];
	static float power;
	
	memset(buf, 0, sizeof(buf));
	bytes = recv(clientfd, buf, sizeof(buf), 0);
	if (bytes <= 0)
		return;

	if (buf[bytes - 1] == '\n'){
		printTimely( stdout, "Server receive message from socket %d : %s", clientfd, buf);
	}
	else
		printTimely( stdout, "Server receive message from socket %d : %s\n", clientfd, buf);

	pos = Search_Hash(sock_hash, clientfd);
	pos = Search_Hash(addr_hash, sock_to_addr[pos]);

	if (buf[0] == '&') {
		float sens = strtof(buf + 1, 0);
		addr_data[pos].sens = sens;
		printTimely( stdout, "Client's sensibility has been changed to %f\n", sens);
	}	
	else{
		float power2;
		char *p;
		Unpacking(buf, &p, &power);
		bytes -= p - buf;
		while (bytes > 0) {
			for(int i = 0; i < HASH_CONSTANT; i++){
				power2 = Power(addr_data[i].x, addr_data[i].y, addr_data[pos].x, addr_data[pos].y, power, freq);
				if (i != pos && addr_data[i].isPresent && power2 > addr_data[i].sens) {
					sprintf(msg, "%f %s", power2, p);
					send(addr_data[i].sock, msg, sizeof(msg), 0);
					printTimely( stdout, "Client %d send message to client %d", sock_to_addr[Search_Hash(sock_hash, clientfd)], addr_hash[i]);
				}
			}
			
			bytes = recv(clientfd, buf, sizeof(buf), MSG_DONTWAIT);
			p = buf;	
		}
	}
}

void ClientRegister(int sock, int pollfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], int sock_to_addr[]) {
	static struct epoll_event ev;
	int pos;
	int newsock = accept(sock, 0,0);
	if (newsock == -1)
		Die(sock, "ERROR: accept() failed");

	printTimely( stdout, "Socket %d is added\n", newsock);

	//receiving of node's address
	static char buf[ADDR_SIZE], msg[ERRMSG_SIZE];
	int addr, index;
	while (1){
		memset(buf, 0, sizeof(buf));
		if( 0 == read(newsock, (void *) buf, sizeof(buf)) ) {
			shutdown(newsock, SHUT_RDWR);
			close(newsock);
			return;
		}
		addr = atoi(buf);
		pos = Search_Hash(addr_hash, addr);
		if (pos > -1 && !addr_data[pos].isPresent) {
			addr_data[pos].isPresent = true;
			addr_data[pos].sock = newsock;
			index = Add_Hash(sock_hash, newsock);
			sock_to_addr[index] = addr;		
			printTimely( stdout, "Client %d has been registered\n", addr);
			break;
		} else {
			if (pos == -1){
				printTimely( stdout, "%d is invalid address\n", addr);
				sprintf(msg, "you have invalid address\n");
			} else {
				printTimely( stdout, "%d has already registered\n", addr);
				sprintf(msg, "this address was registered\n");
			}
			send(newsock, msg, strlen(msg), 0);
		}
	}
	
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = newsock;
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, newsock, &ev) == -1)
		Die(sock, "ERROR: ClientRegister::epoll_ctl() failed");
}

void ClientUnregister(int sock, int pollfd, int clientfd, int addr_hash[], AddrData_T addr_data[], int sock_hash[], int sock_to_addr[]) {
	static struct epoll_event ev;
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, clientfd, &ev) == -1)
		Die(sock, "ERROR: ClientUnregister::epoll_ctl() failed"); 

	int index = Delete_Hash(sock_hash, clientfd);
	int addr = sock_to_addr[index];

	index = Search_Hash(addr_hash, addr);
	addr_data[index].isPresent = false;

	printTimely( stdout, "Client %d has been unregistered\n", addr);

	shutdown(clientfd, SHUT_RDWR);
	close(clientfd);
}

void Task(int sock, int pollfd, int addr_hash[], AddrData_T addr_data[], float freq) {
	struct epoll_event ev, events[MAX_CLIENTS];
	int nfds, n, newsock;		

	int sock_hash[HASH_CONSTANT], sock_to_addr[HASH_CONSTANT];
	Init_Hash(sock_hash);	

	for( ; ; ) {
		nfds = epoll_wait(pollfd, events, MAX_CLIENTS, -1);
		if(nfds == -1)
			Die(sock, "ERROR: epoll_wait() failed");
		
		for (n = 0; n < nfds; ++n)
			if (events[n].data.fd == sock)
				ClientRegister(sock, pollfd, addr_hash, addr_data, sock_hash, sock_to_addr);
			else {
				if (events[n].events & EPOLLIN)
					TransmitData(events[n].data.fd, addr_hash, addr_data, sock_hash, sock_to_addr, freq);	
				if (events[n].events & (EPOLLHUP | EPOLLERR)) 
					ClientUnregister(sock, pollfd, events[n].data.fd, addr_hash, addr_data, sock_hash, sock_to_addr);
			}		
	}
}

void Deinit(int sock, struct sockaddr_un stSockAddr, int pollfd) {
	shutdown(sock, SHUT_RDWR);
	close(sock);
	unlink(stSockAddr.sun_path);
	
	close(pollfd);
}

int main(void) {
	AddrData_T	addr_data[ HASH_CONSTANT ] = { 0 };
	int			addr_hash[ HASH_CONSTANT ] = { 0 },
				pollfd, sock;
	SockAddr_T	stSockAddr;
	float		freq;
	char		socketFilename[ SOCK_FLNM_SZ ] = { 0 };

	readConfig( addr_hash, addr_data, &freq, socketFilename );
	serverInit( &sock, &stSockAddr, &pollfd, socketFilename );
	Task( sock, pollfd, addr_hash, addr_data, freq );
	Deinit( sock, stSockAddr, pollfd );

	return 0;
}
