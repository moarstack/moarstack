#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define MAX_CLIENTS 10
#define BUF_SIZE 256
#define MSG_SIZE 256
#define ERRMSG_SIZE 256
#define POWER_CONSTANT 63
#define HASH_CONSTANT 4096
#define HASH_EMPTY -1
#define HASH_DELETED -2

//const int MAX_CLIENTS = 10;
//const int STR_SIZE = 256;

int Die(int sock, const char *str) {
	perror(str);
	close(sock);
	exit(EXIT_FAILURE);
}

void Init_Hash(int H[]){	
	int i;	
	for(i = 0; i < HASH_CONSTANT; i++)
		H[i] = HASH_EMPTY;
}

int Hash(int data){
	int key = data;
	key = (key + ~(key << 16)) % HASH_CONSTANT;
	key = (key ^ (key >> 5)) % HASH_CONSTANT;
	key = (key + (key << 3)) % HASH_CONSTANT;
	key = (key ^ (key >> 13)) % HASH_CONSTANT;
	key = (key + ~(key << 9)) % HASH_CONSTANT;
	key = (key ^ (key >> 17)) % HASH_CONSTANT;
	return key;
} 

int Rehash(int pos){
	return (pos + 1) % HASH_CONSTANT;
}

int Search_Hash(int H[], int data){
	int key = Hash(data);
	while(1){
		if (H[key] == HASH_EMPTY)
			return -1;
		if (H[key] == data)
			return key;
		key = Rehash(key);
	}
}

int  Add_Hash(int H[], int data){
	if (Search_Hash(H, data) != -1)
		return -1;
	int key = Hash(data);
	while(1){
		if (H[key] == HASH_EMPTY || H[key] == HASH_DELETED){
			H[key] = data;
			return key;
		}
		key = Rehash(key);
	}
}

void Delete_Hash(int H[], int data){
	int key = Hash(data);
	while(1){
		if (H[key] == HASH_EMPTY)
			return;
		if (H[key] == data){
			H[key] = HASH_DELETED;
			return;
		}
		key = Rehash(key);
	}			
}

void ReadConfig(int Address[], double X[], double Y[]) {
	FILE *in;
	in = fopen("config.txt", "r");

	Init_Hash(Address);
	
	int n;
	fscanf(in, "%d\n", &n);
	
	int i, index, addr;
	double x, y;
	for (i = 0; i < n; i++){
		fscanf(in, "%d%lf%lf\n", &addr, &x, &y);
		index = Add_Hash(Address, addr);
		X[index] = x;
		Y[index] = y;
	}	
	
	fclose(in);
}

void Init(int *sock, struct sockaddr_un *stSockAddr, int *pollfd) {
	*sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (*sock == -1)
		Die(0, "socket failed");
	
	memset(stSockAddr, 0, sizeof(struct sockaddr_un));
	stSockAddr->sun_family=AF_UNIX;
	realpath(".", stSockAddr->sun_path);
	strcat(stSockAddr->sun_path, "/socket");
	unlink (stSockAddr->sun_path);
	
	if(bind(*sock, (struct sockaddr*) stSockAddr, sizeof(*stSockAddr)) == -1)
		Die(*sock, "Ошибка: связывания");
		
	if (listen(*sock, 1) == -1)
		Die(*sock, "Ошибка: прослушивания");

	*pollfd = epoll_create(MAX_CLIENTS);
	if (*pollfd == -1)
		Die(*sock, "epoll_create");
	
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = *sock;
	if (epoll_ctl(*pollfd, EPOLL_CTL_ADD, *sock, &ev) == -1)
		Die(*sock, "epoll_ctl: sock");
}

double Distance(double x1, double y1, double x2, double y2) {
	register double dx = x2 - x1, dy = y2 - y1;
	return dx * dx + dy * dy;
}

double Power(double x1, double y1, double x2, double y2, double start_power) {
	return (start_power - POWER_CONSTANT * Distance(x1, y1, x2, y2));
}

void Unpacking(char buf[], char msg[], double* power) {
	char *p;
	*power = strtod(buf, &p);
		
	while (*p == ' ')
		++p;

	strcpy(msg, p);
}

int FindPos(int clientfd, int nclients, int clients[]) {
	int i;
	for(i = 0; i < nclients; i++)
		if (clients[i] == clientfd)
			return i;

	return -1; 
}

void TransmitData(int clientfd, int Address[], bool flag[], int sock_hash[], int sock_to_addr[], int addr_to_sock[], double X[], double Y[]) {
	static char buf[BUF_SIZE], msg[MSG_SIZE], errmsg[ERRMSG_SIZE];
	static double power;
	
	memset(&buf, 0, sizeof(buf));
	recvfrom(clientfd, buf, sizeof(buf), 0,0,0);
        printf("%s\n", buf);

	if (buf[0] == 'a'){
		int addr = strtol(buf + 1, 0, 0);
		int pos = Search_Hash(Address, addr);
		if (pos > -1) {
			flag[pos] = true;
			addr_to_sock[pos] = clientfd;
		}
		
		int index = Search_Hash(sock_hash, clientfd);
		sock_to_addr[index] = addr;		

		printf("\nClient %d has been registered\n", addr);		
	}	
	else{
		memset(&msg, 0, sizeof(msg));
		Unpacking(buf, msg, &power);

		int pos = Search_Hash(sock_hash, clientfd);
		pos = Search_Hash(Address, sock_to_addr[pos]);	
		
		int i;
		for(i = 0; i < HASH_CONSTANT; i++)
			if (i != pos && flag[i] /*&& Power(X[i], Y[i], X[pos], Y[pos], power) > 0*/) {
				send(addr_to_sock[i], msg, sizeof(msg), 0);
				printf("\nClient %d send message to client %d\n", sock_to_addr[Search_Hash(sock_hash, clientfd)], Address[i]);
			}
	}
}

void ClientRegister(int sock, int pollfd, int sock_hash[]) {
	static struct epoll_event ev;
	int newsock = accept(sock, 0,0);
	if (newsock == -1)
		Die(sock, "Ошибка: принятия");

	int pos = Add_Hash(sock_hash, newsock);

	printf("\nSocket %d is added\n", newsock);
	printf("pos[socket] == %d\n", pos);

	//setnonblocking(newsock);                		
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = newsock;
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, newsock, &ev) == -1)
		Die(sock, "epoll_ctl_add: newsock");
}

void ClientUnregister(int sock, int pollfd, int clientfd, int sock_hash[], int sock_to_addr[], int Address[], bool flag[]) {
	static struct epoll_event ev;
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, clientfd, &ev) == -1)
		Die(sock, "epoll_ctl_del: newsock"); 

	int index = Search_Hash(sock_hash, clientfd);
	sock_hash[index] = HASH_DELETED;

	int addr = sock_to_addr[index];
	index = Search_Hash(Address, addr);
	flag[index] = false;

	printf("\nClient %d has been unregistered\n", addr);

	shutdown(clientfd, SHUT_RDWR);
	close(clientfd);
}

void Task(int sock, int pollfd, int Address[], double X[], double Y[]) {
	struct epoll_event ev, events[MAX_CLIENTS];
	int nfds, n, newsock;	

	bool flag[HASH_CONSTANT];
	int  addr_to_sock[HASH_CONSTANT];
	memset(&flag, 0, sizeof(flag));
	
	int sock_hash[HASH_CONSTANT], sock_to_addr[HASH_CONSTANT];
	Init_Hash(sock_hash);	

	for( ; ; ) {
		printf("\nDEBUG\n");
		int i;
		for(i = 0; i < HASH_CONSTANT; i++)
			if (flag[i]) {
				printf("Address == %d, addr_to_sock == %d\n", Address[i], addr_to_sock[i]);
			}
		
		nfds = epoll_wait(pollfd, events, MAX_CLIENTS, -1);
		if(nfds == -1)
			Die(sock, "epoll_wait");
		
		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == sock)
				ClientRegister(sock, pollfd, sock_hash);
			else {
				if(events[n].events == EPOLLIN)
					TransmitData(events[n].data.fd, Address, flag, sock_hash, sock_to_addr, addr_to_sock, X, Y);	
				else
					ClientUnregister(sock, pollfd, events[n].data.fd, sock_hash, sock_to_addr, Address, flag);
			}
			
		}
	}
}

void Deinit(int sock, struct sockaddr_un stSockAddr) {
	shutdown(sock, SHUT_RDWR);
	close(sock);
	unlink(stSockAddr.sun_path);
}

int main(void) {
	int Address[HASH_CONSTANT];
	double X[HASH_CONSTANT], Y[HASH_CONSTANT];
	ReadConfig(Address, X, Y);

	printf("pos[123] == %d, pos[5213] == %d\n", Search_Hash(Address, 123), Search_Hash(Address, 5213));	
	
	int pollfd, sock;
	struct sockaddr_un stSockAddr;	
	Init(&sock, &stSockAddr, &pollfd);
	
	Task(sock, pollfd, Address, X, Y);

	Deinit(sock, stSockAddr);

	return 0; 
}
