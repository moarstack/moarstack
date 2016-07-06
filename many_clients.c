#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAX_EVENTS 10
#define STR_SIZE 256

//const int MAX_EVENTS = 10;
//const int STR_SIZE = 256;

int Die(int sock, const char *str) {
	perror(str);
	close(sock);
	exit(EXIT_FAILURE);
}

void Init(int *sock, struct sockaddr_un *stSockAddr, int *pollfd) {
	*sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (*sock == -1)
		Die(0, "socket failed");
	
	memset(stSockAddr, 0, sizeof(*stSockAddr));
	stSockAddr->sun_family=AF_UNIX;
	realpath(".", stSockAddr->sun_path);
	strcat(stSockAddr->sun_path, "/socket");
	unlink (stSockAddr->sun_path);
	
	if(bind(*sock, (struct sockaddr*) stSockAddr, sizeof(*stSockAddr)) == -1)
		Die(*sock, "Ошибка: связывания");
		
	if (listen(*sock, 1) == -1)
		Die(*sock, "Ошибка: прослушивания");

	*pollfd = epoll_create(10);
	if (*pollfd == -1)
		Die(*sock, "poll_create");
	
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = *sock;
	if (epoll_ctl(*pollfd, EPOLL_CTL_ADD, *sock, &ev) == -1)
		Die(*sock, "poll_ctl: sock"); 
}

void Unpacking(char buf[], int *naim, int aim[], char msg[]) {
	*naim = 0;
	int i;
        for(i = 0; '0' <= buf[i] && buf[i] <= '9'; i++)
        	*naim = 10 * (*naim) + buf[i] - '0';

        int j = 0;
        for(; buf[i] && j < *naim; i++)
        	if ('0' <= buf[i] && buf[i] <= '9') {
                	aim[j] = 0;
                        while (buf[i] && '0' <= buf[i] && buf[i] <= '9') {
                        	aim[j] = 10 * aim[j] + buf[i] - '0';
                                ++i;
                        }
                        ++j;
                }

        strcpy(msg, &buf[i]);
}

void TransmitData(int clientfd, int nclients, int clients[]) {
	static char buf[STR_SIZE], msg[STR_SIZE], errmsg[STR_SIZE];
	static int naim = 0, aim[MAX_EVENTS];
	
	memset(&buf, 0, sizeof(buf));
	recvfrom(clientfd, buf, sizeof(buf), 0,0,0);
        printf("%s\n", buf);
        
	memset(&msg, 0, sizeof(msg));
	Unpacking(buf, &naim, aim, msg);
	
	int i;
	for(i = 0; i < naim; i++)
		if (aim[i] < nclients)
			send(clients[aim[i]], msg, sizeof(msg), 0);
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
			clients[i] = clients[--(*nclients)];
			break;
		}

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
	struct epoll_event ev, events[MAX_EVENTS];
	int nfds, n, newsock;	
	int nclients = 0, clients[MAX_EVENTS];

	for( ; ; ) {
		//printf("clients: ");
		//Write(nclients, clients);
		nfds = epoll_wait(pollfd, events, MAX_EVENTS, -1);
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
