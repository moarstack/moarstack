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

#define MIN(x,y) (((x)>(y))?(y):(x))
#define BUFFER_SIZE_INITIAL 1024

char * ReadInputData(int fd, size_t * dataLen) {
	char * buffer = malloc(BUFFER_SIZE_INITIAL);
	char * storage = malloc(BUFFER_SIZE_INITIAL);
	char * offset = storage;
	size_t storageSize = BUFFER_SIZE_INITIAL;
	int done = 0;
	size_t readLen = 0;
	while ((readLen = read(fd, buffer, BUFFER_SIZE_INITIAL)) > 0) {
		// if not possible to place, reallocate storage
		if (readLen > storageSize - (offset - storage)) {
			//need to reallocate 
			storage = realloc(storage, storageSize * 2);
			storageSize *= 2;
		}
		memcpy(offset, buffer, readLen);
		offset += readLen;
	}
	*dataLen = offset - storage;
	free(buffer);
	return storage;
}

typedef struct {
	int verbose;
	AppId_T OwnAppId;
	AppId_T RemoteAppId;
	RouteAddr_T RemoteAddr;
	char * SocketPath;
} CliArgs_T;

void ShowUsage() {
	puts("moarnc usage:\n");
	puts("-v\t--verbose\tVerbose otput");
	puts("-s\t--socket\tSpecify custom file socket to interact with MOAR stack");
	puts("-p\t--appid\t\tRemote application id to send data");
	puts("-h\t--remote\tRemote node address to send data");
	puts("-l\t--listen\tLocal application id");
}

static struct option long_options[] = {
	{"verbose", no_argument,       0, 'v'}, //verbosity mode
	{"socket",  required_argument, 0, 's'}, //custom socket
	{"appid",   required_argument, 0, 'p'}, //Remote AppId
	{"remote",  required_argument, 0, 'h'}, //Remore Address
	{"listen",  required_argument, 0, 'l'}, //Own AppId
	{0, 0, 0, 0}
};

int parseArgs(int argc, char *argv[], CliArgs_T *CliArgs) {
	int optchar, optindex;
	while ((optchar = getopt_long(argc, argv, "vs:p:h:l:", long_options, &optindex)) != -1) {
		int intValue;
		switch (optchar) {
			case 'v':
				CliArgs->verbose = true;
				break;
			case 's':
				CliArgs->SocketPath = optarg;
				fprintf (stderr , "NOTE: using socket path \"%s\"\n", optarg);
				break;
			case 'p':
				if (AppIdFromStr(optarg, &CliArgs->RemoteAppId) != FUNC_RESULT_SUCCESS) {
					fprintf(stderr, "ERROR: appid is invalid \"%s\"\n", optarg);
					return FUNC_RESULT_FAILED_ARGUMENT;
				}
				break;
			case 'h':
				if (moarAddrFromStr(optarg, &CliArgs->RemoteAddr) != FUNC_RESULT_SUCCESS) {
					fprintf(stderr, "ERROR: remote address is invalid \"%s\"\n", optarg);
					return FUNC_RESULT_FAILED_ARGUMENT;
				}
				break;
			case 'l':
				if (AppIdFromStr(optarg, &CliArgs->OwnAppId) != FUNC_RESULT_SUCCESS) {
					fprintf(stderr, "ERROR: appid is invalid: \"%s\"\n", optarg);
					return FUNC_RESULT_FAILED_ARGUMENT;
				}
				break;
			case '?':
			default:
				return FUNC_RESULT_FAILED_ARGUMENT;
		}
	}
	return FUNC_RESULT_SUCCESS;
}

int main (int argc, char **argv)  {
	CliArgs_T CliArgs = {0};
	
	if (parseArgs(argc, argv, &CliArgs) != FUNC_RESULT_SUCCESS) {
		ShowUsage();
		return EXIT_FAILURE;
	}
	
	MoarDesc_T * md;
	md = (CliArgs.SocketPath)?moarSocketFile(CliArgs.SocketPath):moarSocket();
	if (!md) {
		fprintf(stderr, "ERROR: Could not open MOARStack socket\n");
		return EXIT_FAILURE;
	}
	if (moarBind(md, &CliArgs.OwnAppId) != FUNC_RESULT_SUCCESS) {
		fprintf(stderr, "ERROR: Could not bind socket to appid %d", (int) CliArgs.OwnAppId);
		return EXIT_FAILURE;
	}
	if (CliArgs.verbose)
		fprintf(stderr, "Socket successfully bint: fd=%d; appid=%d\n", md->SocketFd, CliArgs.OwnAppId);
	
	struct epoll_event events[MAX_EPOLL_EVENTS], event;
	int efd;
	if ((efd = epoll_create(MAX_EPOLL_EVENTS)) == -1) {
		perror("epoll_create");
		return EXIT_FAILURE;
	}
	addToEpoll(efd, STDIN_FILENO);
	addToEpoll(efd, md->SocketFd);
	
	int result = 0;
	while (true) {
		int newFd;
		int n = epoll_wait (efd, events, MAX_EPOLL_EVENTS, -1);
		int i;
		for (i = 0; i < n; i++) {
			event = events[i];
			/*printf("NEW EVENT: %u [%d %d %d %d]\r\n", events[i].events,
				   event.events & EPOLLIN,
				   event.events & EPOLLOUT,
				   event.events & EPOLLERR,
				   event.events & EPOLLHUP
			);*/
			if (event.data.fd == STDIN_FILENO) {
				size_t dataLen = 0;
				char * data = ReadInputData(STDIN_FILENO, &dataLen);
				if (CliArgs.verbose)
					fprintf(stdout, "dataLen = %u; message=\"%s\"\n", dataLen, data);
				
				MessageId_T msgId;
				result = moarSendTo(md, data, dataLen, &CliArgs.RemoteAddr, &CliArgs.RemoteAppId, &msgId);
				
				printf("moarSendTo return value: %d\n", result);
				free(data);
			}
			else if (event.data.fd == md->SocketFd) {
				void * inData;
				result = moarRecvFromRaw(md, &inData, NULL, NULL);
				printf("moarRecvFromRaw return value: %d; message = \"%s\"\n", result, (char *) inData);
				printf("test output\n");
				free(inData);
			}
		}
	}
	moarClose(md);
	return EXIT_SUCCESS;
}
