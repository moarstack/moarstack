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

void ReceiveData(int clientfd) {
	static char buf[STR_SIZE];
	memset(&buf, 0, sizeof(buf));
	recvfrom(clientfd, buf, sizeof(buf), 0,0,0);
        printf("%s\n", buf);
        char msg[STR_SIZE] = "zdraste\n";
        send(clientfd, msg, sizeof(msg), 0);
}

void ClientRegister(int sock, int pollfd) {
	static struct epoll_event ev;
	int newsock = accept(sock, 0,0);
	if (newsock == -1)
		Die(sock, "Ошибка: принятия");

	//setnonblocking(newsock);                		
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = newsock;
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, newsock, &ev) == -1)
		Die(sock, "epoll_ctl_add: newsock");
}

void ClientUnregister(int sock, int pollfd, int clientfd) {
	static struct epoll_event ev;
	if (epoll_ctl(pollfd, EPOLL_CTL_DEL, clientfd, &ev) == -1)
		Die(sock, "epoll_ctl_del: newsock"); 
	else {
		shutdown(clientfd, SHUT_RDWR);
		close(clientfd);
	}
}

void Task(int sock, int pollfd) {
	static struct epoll_event ev, events[MAX_EVENTS];
	int nfds, n, newsock;	

	for( ; ; ) {
		nfds = epoll_wait(pollfd, events, MAX_EVENTS, -1);
		if(nfds == -1)
			Die(sock, "poll_pwait");
		
		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == sock)
				ClientRegister(sock, pollfd);
			else {
				if (events[n].events == EPOLLERR)
					ClientUnregister(sock, pollfd, events[n].data.fd);
				else if(events[n].events == EPOLLIN)
					ReceiveData(events[n].data.fd);
			}
			
		}
	}
}

void Deinit(int sock, struct sockaddr_un stSockAddr) {
	shutdown(sock, SHUT_RDWR);
	close(sock);
	unlink(stSockAddr.sun_path);
}

int main(void){
	int pollfd, sock;
	struct sockaddr_un stSockAddr;	

	Init(&sock, &stSockAddr, &pollfd);
	
	Task(sock, pollfd);

	Deinit(sock, stSockAddr);

	return 0; 
}
