#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "moarApi.h"
#include <getopt.h>
#include <moarService.h>
#include <sys/epoll.h>

#define MAX_EPOLL_EVENTS 100

int addToEpoll(int efd, int newfd) {
    int rc;
    struct epoll_event event = {
            .data.fd = newfd,
            .events = EPOLLIN | EPOLLET
    };
    if ((rc = epoll_ctl (efd, EPOLL_CTL_ADD, newfd, &event)) == -1) {
        perror ("epoll_ctl: add");
        return FUNC_RESULT_FAILED_IO;
    }
    printf("Add new fd to epoll\r\n");
    return FUNC_RESULT_SUCCESS;
}

int AppIdFromStr(const char *str, AppId_T *appid) {
    int intValue;
    if (sscanf(optarg, "%d", &intValue) == 0) {
        fprintf(stderr, "ERROR: appid is invalid\n");
        return FUNC_RESULT_FAILED_ARGUMENT;
    }
    *appid = intValue;
    return FUNC_RESULT_SUCCESS;
}

void DumpRouteAddr(FILE *fp, const RouteAddr_T * addr) {
    int i;
    for (i=0; i<sizeof(RouteAddr_T)/sizeof(addr->Address[0]); i++) {
        fprintf(fp, "%02x", addr->Address[i]);
    }
}

int main (int argc, char **argv)  {
    /* Flag set by ‘--verbose’. */
    static int verbose = 0;
    AppId_T OwnAppId;
    AppId_T RemoteAppId;
    RouteAddr_T RemoteAddr;
    char * CustomSocketPath = NULL;

    static struct option long_options[] = {
            {"verbose", no_argument,      &verbose, 1}, // verbosity mode
            {"socket",  required_argument, 0, 's'}, //custom socket
            {"appid",   required_argument, 0, 'p'}, //Remote AppId to send data to
            {"remote",  required_argument, 0, 'h'}, //Remore Address
            {"listen",  required_argument, 0, 'l'},  // Listening mode; argument is own AppId
            {0, 0, 0, 0}
    };
    int c;
    while (1) {
        int intValue;
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long (argc, argv, "s:p:h:l:v", long_options, &option_index);
        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'v':
                verbose = true;
                break;
            case 's':
                CustomSocketPath = optarg;
                fprintf (stderr , "NOTE: using socket path \"%s\"\n", CustomSocketPath);
                break;
            case 'p':
                if (AppIdFromStr(optarg, &OwnAppId) != FUNC_RESULT_SUCCESS) {
                    fprintf(stderr, "ERROR: appid is invalid \"%s\"\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 'h':
                if (moarAddrFromStr(optarg, &RemoteAddr) != FUNC_RESULT_SUCCESS) {
                    fprintf(stderr, "ERROR: remote address is invalid \"%s\"\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 'l':
                printf ("option -l with value `%s'\n", optarg);
                if (AppIdFromStr(optarg, &RemoteAppId) != FUNC_RESULT_SUCCESS) {
                    fprintf(stderr, "ERROR: appid is invalid: \"%s\"\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort ();
        }
    }

    /* Print any remaining command line arguments (not options). */
    /*if (optind < argc) {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }*/
    MoarDesc_T * md;
    md = (CustomSocketPath)?moarSocketFile(CustomSocketPath):moarSocket();
    if (!md) {
        fprintf(stderr, "ERROR: Could not open MOARStack socket\n");
        return EXIT_FAILURE;
    }
    if (moarBind(md, &OwnAppId) != FUNC_RESULT_SUCCESS) {
        fprintf(stderr, "ERROR: Could not bind socket to appid %d", (int) OwnAppId);
        return EXIT_FAILURE;
    }
    if (verbose)
        fprintf(stderr, "Socket successfully bint: fd=%d; appid=%d\n", md->SocketFd, OwnAppId);

    struct epoll_event events[MAX_EPOLL_EVENTS], event;
    int efd;
    if ((efd = epoll_create(MAX_EPOLL_EVENTS)) == -1) {
        perror("epoll_create");
        return EXIT_FAILURE;
    }
    addToEpoll(efd, STDIN_FILENO);
    addToEpoll(efd, md->SocketFd);
    while (true) {
        int newFd;
        // wait event via epoll
        int n = epoll_wait (efd, events, MAX_EPOLL_EVENTS, -1);
        int i;
        for (i = 0; i < n; i++) {
            event = events[i];
            printf("NEW EVENT: %u [%d %d %d %d]\r\n", events[i].events,
                   event.events & EPOLLIN,
                   event.events & EPOLLOUT,
                   event.events & EPOLLERR,
                   event.events & EPOLLHUP
            );
            if (event.data.fd == STDIN_FILENO) {
                printf("Event from stdin\n");
                //Read data here
                
            }
            else if (event.data.fd == md->SocketFd) {
                printf("Event from moarstack\n");
            }
        }
    }

    moarClose(md);
    return EXIT_SUCCESS;
}
