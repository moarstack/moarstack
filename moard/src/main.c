#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include "localheader.h"
#include "header.h"


#define UNIX_PATH_MAX 256

int connection_handler(int connection_fd)
{
 int nbytes;
 char buffer[256];

 nbytes = read(connection_fd, buffer, 256);
 buffer[nbytes] = 0;

 printf("MESSAGE FROM CLIENT: %s\n", buffer);
 nbytes = snprintf(buffer, 256, "hello from the server");
 write(connection_fd, buffer, nbytes);

 close(connection_fd);
 return 0;
}

char* socket_name = "/home/svalov/demo_socket";

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(void) {
    struct sockaddr_un address;
    int socket_fd, connection_fd;
    socklen_t address_length;
    pid_t child;

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("socket() failed\n");
        return 1;
    }
    printf("listen socket %d\n", socket_fd);
    unlink(socket_name);

    /* start with a clean address structure */
    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    //snprintf(address.sun_path, UNIX_PATH_MAX, "/home/svalov/demo_socket");
    strncpy(address.sun_path, socket_name, sizeof(address.sun_path) - 1);
    printf("Socket %s\n", address.sun_path);

    int status;
    if ((status = bind(socket_fd,
                       (struct sockaddr *) &address,
                       sizeof(struct sockaddr_un))) != 0) {
        printf("bind() failed %d, status %d\n", errno, status);
        return 1;
    }

    if (listen(socket_fd, 5) != 0) {
        printf("listen() failed\n");
        return 1;
    }
    struct pollfd fds[10];
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;
    int fds_count = 1;
    int flags = fcntl(socket_fd, F_GETFL, 0);
    flags = fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    printf("fnctl result %d\n", flags);
    while (1) {
        int val = poll(fds, fds_count, 10000);
        if(val>0){
            printf("polled\n");
            if(fds[0].revents != 0) {
                if ((connection_fd = accept(socket_fd,
                                            (struct sockaddr *) &address,
                                            &address_length)) > -1) {
                    printf("socket %d\n", connection_fd);
                    fds[fds_count].fd = connection_fd;
                    fds[fds_count].events = POLLIN;
                    fds[fds_count].revents = 0;
                    fds_count++;
                }
            }
            for (int i = 1; i < fds_count; i++) {
                if ((fds[i].revents & POLLIN) != 0) {
                    int nbytes;
                    char buffer[256];

                    nbytes = read(fds[i].fd, buffer, 256);
                    buffer[nbytes] = 0;

                    printf("MESSAGE FROM CLIENT %d: %s\n", fds[i].fd, buffer);
                    nbytes = snprintf(buffer, 256, "hello from the server");
                    write(fds[i].fd, buffer, nbytes);
                    fds[i].revents = 0;
                }
            }
        }
    }
    return 0;
}
#pragma clang diagnostic pop
//
//
//
//
//
//
//
//
//
//
//
//
//
//    if((connection_fd = accept(socket_fd,
//                                  (struct sockaddr *) &address,
//                                  &address_length))
//          > -1
//          )
//    {
//   printf("socket %d\n",connection_fd);
//   child = fork();
//   if(child == 0)
//   {
//    /* now inside newly created connection handling process */
//    return connection_handler(connection_fd);
//   }
//
//   /* still inside server process */
//   close(connection_fd);
//  }
//  usleep(10);
// }
//
// close(socket_fd);
// unlink("socket_name");
// return 0;
//}