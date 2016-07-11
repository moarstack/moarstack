#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/param.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "hash.h"
#define CONFIG_FILE "config.txt"
#define MAX_CLIENTS 10
#define BUF_SIZE 256
#define MSG_SIZE 256
#define ERRMSG_SIZE 256
#define ADDR_SIZE 30
#define POWER_CONSTANT 63
#define PI 3.14159265358
#define LIGHT_SPEED 299792458

struct AddrData {
	bool isPresent;
	int sock;
	float x, y, sens;
};

char *GetTime(void) {
	static char buf[30];
	time_t sec = time(NULL);
	struct tm *pt = localtime(&sec);
	strcpy(buf, asctime(pt));

	char *p = strchr(buf, '\n');
	if (p)
		*p = '\0';	

	return buf;
}

int Die(int sock, const char *str) {
	perror(GetTime());
	perror(" : ");
	perror(str);
	close(sock);
	exit(EXIT_FAILURE);
}

void ReadConfig(int addr_hash[], struct AddrData addr_data[], float *freq) {
	FILE *in;
	in = fopen(CONFIG_FILE, "r");

	Init_Hash(addr_hash);
	
	int n;
	fscanf(in, "%f%d\n", freq, &n);
	
	int index, addr;
	float x, y, sens;
	for (int i = 0; i < n; i++){
		fscanf(in, "%d%f%f%f\n", &addr, &x, &y, &sens);
		index = Add_Hash(addr_hash, addr);
		addr_data[index].x = x;
		addr_data[index].y = y;
		addr_data[index].sens = sens;
		addr_data[index].isPresent = false;
	}	
	
	fclose(in);
}

void Init(int *sock, struct sockaddr_un *stSockAddr, int *pollfd) {
	*sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (*sock == -1)
		Die(0, "ERROR: socket() failed");
	
	memset(stSockAddr, 0, sizeof(struct sockaddr_un));
	stSockAddr->sun_family = AF_UNIX;
	realpath("./socket", stSockAddr->sun_path);
	unlink (stSockAddr->sun_path);
	
	if(bind(*sock, (struct sockaddr*) stSockAddr, sizeof(*stSockAddr)) == -1)
		Die(*sock, "ERROR: bind() failed");
		
	if (listen(*sock, 1) == -1)
		Die(*sock, "ERROR: listen() failed");

	*pollfd = epoll_create(MAX_CLIENTS);
	if (*pollfd == -1)
		Die(*sock, "ERROR: epoll_create() failed");
	
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = *sock;
	if (epoll_ctl(*pollfd, EPOLL_CTL_ADD, *sock, &ev) == -1)
		Die(*sock, "ERROR: Init::epoll_ctl() failed");
}

float Distance(float x1, float y1, float x2, float y2) {
	register float dx = x2 - x1, dy = y2 - y1;
	return dx * dx + dy * dy;
}

float Power(float x1, float y1, float x2, float y2, float start_power, float freq) {
	float free_att = 20.0 * log10f(4.0 * PI * Distance(x1, y1, x2, y2) / (LIGHT_SPEED / freq));
	float res = start_power - free_att;
	if (start_power < res) res = start_power;
	return res;  
}

void Unpacking(char buf[], char msg[], float* power) {
	char *p;
	*power = strtof(buf, &p);
		
	while (*p == ' ')
		++p;

	strcpy(msg, p);
}

void TransmitData(int clientfd, int addr_hash[], struct AddrData addr_data[], int sock_hash[], int sock_to_addr[], float freq) {
	int pos;
	static char buf[BUF_SIZE], msg[MSG_SIZE];
	static float power;
	
	memset(buf, 0, sizeof(buf));
	recvfrom(clientfd, buf, sizeof(buf), 0,0,0);

	int l = strlen(buf);
	if (buf[l - 1] == '\n')
		buf[l - 1] = '\0';

	printf("%s : ", GetTime());
        printf("Server receive message from socket %d : %s\n", clientfd, buf);

	pos = Search_Hash(sock_hash, clientfd);
	pos = Search_Hash(addr_hash, sock_to_addr[pos]);

	if (buf[0] == '&') {
		float sens = strtof(buf + 1, 0);
		addr_data[pos].sens = sens;
		printf("%s : ", GetTime());	
		printf("Client's sensibility has been changed to %f\n", sens);
	}	
	else {
		memset(&msg, 0, sizeof(msg));
		Unpacking(buf, msg, &power);
 	
		for(int i = 0; i < HASH_CONSTANT; i++)
			if (i != pos && addr_data[i].isPresent && Power(addr_data[i].x, addr_data[i].y, addr_data[pos].x, addr_data[pos].y, power, freq) > addr_data[i].sens) {
				send(addr_data[i].sock, msg, sizeof(msg), 0);
				printf("%s : ", GetTime());
				printf("Client %d send message to client %d\n", sock_to_addr[Search_Hash(sock_hash, clientfd)], addr_hash[i]);
		}
	}
}

void ClientRegister(int sock, int pollfd, int addr_hash[], struct AddrData addr_data[], int sock_hash[], int sock_to_addr[]) {
	static struct epoll_event ev;
	int pos;
	int newsock = accept(sock, 0,0);
	if (newsock == -1)
		Die(sock, "ERROR: accept() failed");

	
	printf("%s : ", GetTime());
	printf("Socket %d is added\n", newsock);

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
			printf("%s : ", GetTime());
			printf("Client %d has been registered\n", addr);	
			break;
		} else {
			if (pos == -1){
				printf("%s : ", GetTime());
				printf("%d is invalid address\n", addr);
				sprintf(msg, "you have invalid address\n");
			} else {
				printf("%s : ", GetTime());
				printf("%d has already registered\n", addr);
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

void ClientUnregister(int sock, int pollfd, int clientfd, int addr_hash[], struct AddrData addr_data[], int sock_hash[], int sock_to_addr[]) {
	static struct epoll_event ev;
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, clientfd, &ev) == -1)
		Die(sock, "ERROR: ClientUnregister::epoll_ctl() failed"); 

	int index = Delete_Hash(sock_hash, clientfd);
	int addr = sock_to_addr[index];

	index = Search_Hash(addr_hash, addr);
	addr_data[index].isPresent = false;

	printf("%s : ", GetTime());
	printf("Client %d has been unregistered\n", addr);

	shutdown(clientfd, SHUT_RDWR);
	close(clientfd);
}

void Task(int sock, int pollfd, int addr_hash[], struct AddrData addr_data[], float freq) {
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
	int addr_hash[HASH_CONSTANT];
	struct AddrData addr_data[HASH_CONSTANT];
	float freq;
	ReadConfig(addr_hash, addr_data, &freq);
	
	int pollfd, sock;
	struct sockaddr_un stSockAddr;	
	Init(&sock, &stSockAddr, &pollfd);
	
	Task(sock, pollfd, addr_hash, addr_data, freq);

	Deinit(sock, stSockAddr, pollfd);

	return 0; 
}
