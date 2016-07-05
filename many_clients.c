#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket failed");
		return EXIT_FAILURE;
	}

	struct sockaddr_un stSockAddr;
	
	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sun_family = AF_UNIX;
	realpath(".", stSockAddr.sun_path);
	strcat(stSockAddr.sun_path, "/socket");
	unlink(stSockAddr.sun_path);

	if (bind(sock, (struct sockaddr*) &stSockAddr, sizeof(stSockAddr)) == -1) {
		perror("Ошибка: связывания");

                close(sock);
                return EXIT_FAILURE;
	}

	if (listen(sock, 1) == -1) {
                perror("Ошибка: прослушивания");

                close(sock);
                return EXIT_FAILURE;
	}

	int newsock = accept(sock, 0, 0);
	if (newsock == -1) {
	        perror("Ошибка: принятия");
                close(sock);
                return EXIT_FAILURE;
	}

	char buf[256];
	int i;
	for(i = 0; i < 255; i++)
		buf[i] = 0;	

	recvfrom(newsock, buf, sizeof(buf), 0, 0, 0);
	printf("%s\n", buf);

	char msg[] = "Hello from server\n";
	send(newsock, msg, sizeof(msg), 0);

	shutdown(sock, SHUT_RDWR);
	shutdown(newsock, SHUT_RDWR);

        close(newsock);
	close(sock);

	unlink(stSockAddr.sun_path);

	return 0;
}
