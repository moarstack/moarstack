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
#include <math.h>
#define MAX_CLIENTS 10
#define BUF_SIZE 256
#define MSG_SIZE 256
#define ERRMSG_SIZE 256
#define ADDR_SIZE 30
#define POWER_CONSTANT 63
#define HASH_CONSTANT 4096
#define HASH_EMPTY -1
#define HASH_DELETED -2
#define CONFIG_FILE "config.txt"
#define PI 3.14159265358
#define LIGHT_SPEED 299792458

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

int Delete_Hash(int H[], int data){
	int key = Hash(data);
	while(1){
		if (H[key] == HASH_EMPTY)
			return -1;
		if (H[key] == data){
			H[key] = HASH_DELETED;
			return key;
		}
		key = Rehash(key);
	}			
}

void ReadConfig(int Address[], float X[], float Y[], float Sensibility[], float *freq ) {
	FILE *in;
	in = fopen(CONFIG_FILE, "r");

	Init_Hash(Address);
	
	int n;
	fscanf(in, "%f%d\n", freq, &n);
	
	int i, index, addr;
	float x, y, sens;
	for (i = 0; i < n; i++){
		fscanf(in, "%d%f%f%f\n", &addr, &x, &y, &sens);
		index = Add_Hash(Address, addr);
		X[index] = x;
		Y[index] = y;
		Sensibility[index] = sens;
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

void TransmitData(int clientfd, int Address[], bool flag[], int sock_hash[], int sock_to_addr[], int addr_to_sock[], float X[], float Y[], float Sensibility[], float freq) {
	static char buf[BUF_SIZE], msg[MSG_SIZE];
	static float power;
	
	memset(&buf, 0, sizeof(buf));
	recvfrom(clientfd, buf, sizeof(buf), 0,0,0);
        printf("\nServer receive message from socket %d : %s\n", clientfd, buf);

	memset(&msg, 0, sizeof(msg));
	Unpacking(buf, msg, &power);
 
	int pos = Search_Hash(sock_hash, clientfd);
	pos = Search_Hash(Address, sock_to_addr[pos]);	
		
	int i;
	for(i = 0; i < HASH_CONSTANT; i++)
		if (i != pos && flag[i] && Power(X[i], Y[i], X[pos], Y[pos], power, freq) > Sensibility[i]) {
			send(addr_to_sock[i], msg, sizeof(msg), 0);
			printf("\nClient %d send message to client %d\n", sock_to_addr[Search_Hash(sock_hash, clientfd)], Address[i]);
		}
}

void ClientRegister(int sock, int pollfd, int sock_hash[], int sock_to_addr[], int Address[], int addr_to_sock[], bool flag[]) {
	static struct epoll_event ev;
	int newsock = accept(sock, 0,0);
	if (newsock == -1)
		Die(sock, "Ошибка: принятия");

	int pos = Add_Hash(sock_hash, newsock);

	printf("\nSocket %d is added\n", newsock);

	//receiving of node's address
	static char buf[ADDR_SIZE], msg[ERRMSG_SIZE];
	while (1){
		memset(&buf, 0, sizeof(buf));
		read(newsock, (void *) buf, sizeof(buf));
		int addr = atoi(buf);
		int pos = Search_Hash(Address, addr);
		if (pos > -1 && !flag[pos]) {
			flag[pos] = true;
			addr_to_sock[pos] = newsock;
			int index = Search_Hash(sock_hash, newsock);
			sock_to_addr[index] = addr;		
			printf("\nClient %d has been registered\n", addr);	
			break;
		}
		else{
			if (pos == -1){
				printf("%d is invalid address\n", addr);
				sprintf(msg, "you have invalid address\n");
				send(newsock, msg, strlen(msg), 0);
			}
			else{
				printf("%d has already registered\n", addr);
                                sprintf(msg, "this address was registered\n");
                                send(newsock, msg, strlen(msg), 0);
			}
		}
	}
	
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = newsock;
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, newsock, &ev) == -1)
		Die(sock, "epoll_ctl_add: newsock");
}

void ClientUnregister(int sock, int pollfd, int clientfd, int sock_hash[], int sock_to_addr[], int Address[], bool flag[]) {
	static struct epoll_event ev;
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, clientfd, &ev) == -1)
		Die(sock, "epoll_ctl_del: newsock"); 

	int index = Delete_Hash(sock_hash, clientfd);
	int addr = sock_to_addr[index];

	index = Search_Hash(Address, addr);
	flag[index] = false;

	printf("\nClient %d has been unregistered\n", addr);

	shutdown(clientfd, SHUT_RDWR);
	close(clientfd);
}

void Task(int sock, int pollfd, int Address[], float X[], float Y[], float Sensibility[], float freq) {
	struct epoll_event ev, events[MAX_CLIENTS];
	int nfds, n, newsock;	

	bool flag[HASH_CONSTANT];
	int  addr_to_sock[HASH_CONSTANT];
	memset(&flag, 0, sizeof(flag));
	
	int sock_hash[HASH_CONSTANT], sock_to_addr[HASH_CONSTANT];
	Init_Hash(sock_hash);	

	for( ; ; ) {
		/*printf("\nDEBUG\n");
		int i;
		for(i = 0; i < HASH_CONSTANT; i++)
			if (flag[i]) {
				printf("Address == %d, addr_to_sock == %d\n", Address[i], addr_to_sock[i]);
			}*/
		nfds = epoll_wait(pollfd, events, MAX_CLIENTS, -1);
		if(nfds == -1)
			Die(sock, "epoll_wait");
		
		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == sock)
				ClientRegister(sock, pollfd, sock_hash, sock_to_addr, Address, addr_to_sock, flag);
			else {
				if (events[n].events == EPOLLIN)
					TransmitData(events[n].data.fd, Address, flag, sock_hash, sock_to_addr, addr_to_sock, X, Y, Sensibility, freq);	
				else 
					ClientUnregister(sock, pollfd, events[n].data.fd, sock_hash, sock_to_addr, Address, flag);
			}
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
	int Address[HASH_CONSTANT];
	float X[HASH_CONSTANT], Y[HASH_CONSTANT], Sensibility[HASH_CONSTANT];
	float freq;
	ReadConfig(Address, X, Y, Sensibility, &freq);
	
	int pollfd, sock;
	struct sockaddr_un stSockAddr;	
	Init(&sock, &stSockAddr, &pollfd);
	
	Task(sock, pollfd, Address, X, Y, Sensibility, freq);

	Deinit(sock, stSockAddr, pollfd);

	return 0; 
}
