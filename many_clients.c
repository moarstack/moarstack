#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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

int Hash(int data){
	int key = data;
	key = (key + ~(key << 16))%HASH_CONSTANT;
	key = (key ^ (key >> 5))%HASH_CONSTANT;
	key = (key + (key << 3))%HASH_CONSTANT;
	key = (key ^ (key >> 13))%HASH_CONSTANT;
	key = (key + ~(key << 9))%HASH_CONSTANT;
	key = (key ^ (key >> 17))%HASH_CONSTANT;
	return key;
} 

int Rehash(int pos){
	return (pos + 1)%HASH_CONSTANT;
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

void Init(int *sock, struct sockaddr_un *stSockAddr, int *pollfd, int X[], int Y[], int Address[]) {
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

	int n;
	int addr, x, y ;
	FILE *in;
	in = fopen(in, "config.txt");
	fscanf(in, "%d\n", &n);
	int i;
	for (i=0; i<n; i++){
		fscanf(in, "%d%d%d\n", &addr, &x, &y);
		int index=Add_Hash(Address, addr);
		X[index]=x;
		Y[index]=y;
	}	
	fclose(in);
}

double Distance(double x1, double y1, double x2, double y2){
	return (x1-x2) * (x1-x2) + (y1 - y2) * (y1 - y2);
}

double Power(double x1, double y1, double x2, double y2, double start_power ){
	return (start_power - POWER_CONSTANT * Distance(x1, y1, x2, y2));
}


void Unpacking(char buf[],  char msg[], float* power) {
	char *p;
	*power = strtolf(buf, &p, 0);
		
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

void TransmitData(int clientfd, int nclients, int clients[]) {
	static char buf[BUF_SIZE], msg[MSG_SIZE], errmsg[ERRMSG_SIZE];
	static float power;
	
	memset(&buf, 0, sizeof(buf));
	recvfrom(clientfd, buf, sizeof(buf), 0,0,0);
        //printf("%s\n", buf);
	if (buf[0] == 'a'){
		int addr = strtol(buf + 1, 0, 0);
		int pos=Search_Hash(Address, addr);
		if (pos > -1)
			flag[pos] = true;
		Add_Hash(Hash_Socket, clientfd);		
	}	
	else{
		memset(&msg, 0, sizeof(msg));
		Unpacking(buf, msg, &power);
	}
	int i;
	for(i = 0; i < naim; i++)
		if (aim[i] < nclients) {
			printf("\nClient %d with address %d\n", clientfd, FindPos(clientfd, nclients, clients)); 
			printf("send message: %s", msg);
			printf("to client %d with address %d\n", clients[aim[i]], aim[i]);
			send(clients[aim[i]], msg, sizeof(msg), 0);		
		}
		else {
			sprintf(errmsg, "Address %d doesn't exist\n", aim[i]);
			send(clientfd, errmsg, sizeof(errmsg), 0);
		}
}

void ClientRegister(int sock, int pollfd, int *nclients, int clients[]) {
	static struct epoll_event ev;
	int newsock = accept(sock, 0,0);
	if (newsock == -1)
		Die(sock, "Ошибка: принятия");

	clients[(*nclients)++] = newsock;	
	printf("\nClient %d has been connected and get address %d\n", newsock, *nclients - 1); 

	//setnonblocking(newsock);                		
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = newsock;
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, newsock, &ev) == -1)
		Die(sock, "epoll_ctl_add: newsock");
}

void ClientUnregister(int sock, int pollfd, int clientfd, int *nclients, int clients[]) {
	static struct epoll_event ev;
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, clientfd, &ev) == -1)
		Die(sock, "epoll_ctl_del: newsock"); 

	int i;
	for(i = 0; i < *nclients; i++)
		if (clients[i] == clientfd) {
			printf("\nClient %d has been unregistered from address %d\n", clientfd, i);
			if (i != *nclients - 1)
				printf("\nClient %d from address %d has gotten new address %d\n", clients[*nclients - 1], (*nclients) - 1, i);
	
			clients[i] = clients[--(*nclients)];
			break;
		}

	Delete_Hash(Hash_Socket, clientfd);
	
	shutdown(clientfd, SHUT_RDWR);
	close(clientfd);
}

/*void Write(int n, int X[]) {
	int i;
	printf("size == %d : ", n);
	for(i = 0; i < n; i++)
		printf("%d ", X[i]);

	printf("\n");
}*/

void Task(int sock, int pollfd) {
	struct epoll_event ev, events[MAX_CLIENTS];
	int nfds, n, newsock;	
	int nclients = 0, clients[MAX_CLIENTS];
	int Address[HASH_CONSTANT];
	int i;
	for(i = 0; i < HASH_CONSTANT; ++i)
		H[i]=-1;
	for( ; ; ) {
		//printf("clients: ");
		//Write(nclients, clients);
		nfds = epoll_wait(pollfd, events, MAX_CLIENTS, -1);
		if(nfds == -1)
			Die(sock, "poll_pwait");
		
		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == sock)
				ClientRegister(sock, pollfd, &nclients, clients);
			else {
				if(events[n].events == EPOLLIN)
					TransmitData(events[n].data.fd, nclients, clients);	
				else
					ClientUnregister(sock, pollfd, events[n].data.fd, &nclients, clients);
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
	int pollfd, sock;
	struct sockaddr_un stSockAddr;	

	Init(&sock, &stSockAddr, &pollfd);
	
	Task(sock, pollfd);

	Deinit(sock, stSockAddr);

	return 0; 
}
