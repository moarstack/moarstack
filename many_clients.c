#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAX_EVENTS 10

int main(void){
	struct epoll_event ev, events[MAX_EVENTS];
	int nfds, pollfd;
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock == -1){
		perror("socket failed" );
		return EXIT_FAILURE;	
	}
	struct sockaddr_un stSockAddr;
	memset(&stSockAddr, 0, sizeof(stSockAddr));
	stSockAddr.sun_family=AF_UNIX;
	realpath(".", stSockAddr.sun_path);
	strcat(stSockAddr.sun_path, "/socket");
	unlink (stSockAddr.sun_path);
	if(bind(sock, (struct sockaddr*) &stSockAddr, sizeof(stSockAddr)) == -1){
		 perror("Ошибка: связывания");

                close(sock);
                return EXIT_FAILURE;
	}
	if (listen(sock, 1) == -1){
		perror("Ошибка: прослушивания");
                close(sock);
                return EXIT_FAILURE;
	}
	pollfd = epoll_create(10);
	if (pollfd == -1){
		perror("poll_create");
		exit (EXIT_FAILURE);
	}
	ev.events = EPOLLIN;
	ev.data.fd = sock;
	if (epoll_ctl(pollfd, EPOLL_CTL_ADD, sock, &ev) == -1){
		perror("poll_ctl: sock");
		exit(EXIT_FAILURE);
	}
	for( ; ; ){
		nfds=epoll_wait(pollfd, events, MAX_EVENTS, -1);
		if(nfds == -1){
			perror("poll_pwait");
			exit(EXIT_FAILURE);
		}
		int n;
		for (n=0; n<nfds; ++n){
			if (events[n].data.fd == sock){		
				int newsock = accept(sock, 0,0);
				if (newsock == -1 ){
                			perror("Ошибка: принятия");
                			close(sock);
                			return EXIT_FAILURE;
        			}
				//setnonblocking(newsock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = newsock;
				if (epoll_ctl(pollfd, EPOLL_CTL_ADD, newsock, &ev) == -1){
					perror("epoll_ctl_add: newsock");
					exit(EXIT_FAILURE);			
				}

		 	}
			else{
				if (events[n].events == EPOLLERR){
					if (epoll_ctl(pollfd, EPOLL_CTL_DEL, events[n].data.fd, &ev) == -1){
                                        	perror("epoll_ctl_del: newsock");
				        	exit(EXIT_FAILURE);
                                	}
					else{
						shutdown(events[n].data.fd, SHUT_RDWR);
						close(events[n].data.fd);
					}
				}
				else if(events[n].events == EPOLLIN){
					char buf[256];
				        int i;
        				for(i=0; i<256; i++)
                			buf[i]=0;
        				recvfrom(events[n].data.fd, buf, sizeof(buf), 0,0,0);
        				printf("%s\n", buf);
        				char msg[] = "zdraste\n";
        				send(events[n].data.fd, msg, sizeof(msg), 0);
				}
			}
			
		}

	}

	shutdown(sock, SHUT_RDWR);
	close(sock);
	unlink(stSockAddr.sun_path);
	return 0; 
}
