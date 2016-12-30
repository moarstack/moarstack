#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "moarApi.h"
#include <getopt.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <time.h>

#define MAX_EPOLL_EVENTS 100
#define BUFFER_SIZE_INITIAL 1024

#ifndef MIN
#define MIN(x,y) (((x)>(y))?(y):(x))
#endif


int sockNonBlock(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) {
		perror("fcntl(F_GETFL)");
		return EXIT_FAILURE;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl(F_SETFL)");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int addToEpoll(int efd, int newfd) {
	struct epoll_event event = {
			.data.fd = newfd,
			.events = EPOLLIN | EPOLLET
	};
	if (epoll_ctl (efd, EPOLL_CTL_ADD, newfd, &event) == -1) {
		perror ("epoll_ctl(EPOLL_CTL_ADD)");
		return FUNC_RESULT_FAILED_IO;
	}
	return FUNC_RESULT_SUCCESS;
}

int AppIdFromStr(const char *str, AppId_T *appid) {
	int intValue;
	if (sscanf(str, "%d", &intValue) == 0) {
		fprintf(stderr, "ERROR: appid is invalid\n");
		return FUNC_RESULT_FAILED_ARGUMENT;
	}
	*appid = (AppId_T)intValue;
	return FUNC_RESULT_SUCCESS;
}

void DumpRouteAddr(FILE *fp, const RouteAddr_T * addr) {
	int i;
	for (i=0; i<sizeof(RouteAddr_T)/sizeof(addr->Address[0]); i++) {
		fprintf(fp, "%02x", addr->Address[i]);
	}
}

void OutMessage(char *msg, size_t len) {
	char *smb;
	for (smb = msg; smb < msg + len; smb++)
		putchar(*smb);
	putchar('\n');
}

//Shrinkable buffer for incoming data
typedef struct {
	char *buffer;
	size_t DataLen;
	size_t StorageSize;
} MessageStorage_T;

MessageStorage_T *MessageStorageCreate(size_t initSize) {
	MessageStorage_T *st = malloc(sizeof(MessageStorage_T));
	st->StorageSize = initSize;
	st->buffer = malloc(st->StorageSize);
	st->DataLen = 0;
	return st;
}
void MessageStorageFree(MessageStorage_T *storage) {
	if (storage->buffer != NULL)
		free(storage->buffer);
	free(storage);
}

size_t ReadInputData(int fd, MessageStorage_T *storage) {
	char * buffer = malloc(BUFFER_SIZE_INITIAL);
	size_t eventReadLen = 0;
	ssize_t readLen = 0;
	while ((readLen = read(fd, buffer, BUFFER_SIZE_INITIAL)) > 0) {
		// if not possible to place, reallocate storage
		if (readLen > storage->StorageSize - eventReadLen) {
			storage->buffer = realloc(storage->buffer, storage->StorageSize * 2);
			storage->StorageSize *= 2;
		}
		memcpy((void *) (storage->buffer + eventReadLen), buffer, (size_t)readLen);
		eventReadLen += readLen;
	}
	storage->DataLen = eventReadLen;
	free(buffer);
	return eventReadLen;
}

typedef struct {
	bool verbose;
	bool listen;
	bool send;
	AppId_T OwnAppId;
	AppId_T RemoteAppId;
	RouteAddr_T RemoteAddr;
	char * SocketPath;
} CliArgs_T;

void ShowUsage() {
	puts("moarnc usage:");
	puts("-v\t--verbose\tVerbose output");
	puts("-h\t--help\t\tPrint this help message");
	puts("-s\t--socket\tSpecify custom file socket to interact with MOAR stack");
	puts("-p\t--appid\t\tRemote application id to send data");
	puts("-r\t--remote\tRemote node address to send data");
	puts("-l\t--listen\tLocal application id");
}

static struct option long_options[] = {
	{"verbose", no_argument,       0, 'v'}, //verbosity mode
	{"help",    no_argument,       0, 'h'}, //show help and exit
	{"socket",  required_argument, 0, 's'}, //custom socket
	{"appid",   required_argument, 0, 'p'}, //Remote AppId
	{"remote",  required_argument, 0, 'r'}, //Remore Address
	{"listen",  required_argument, 0, 'l'}, //Own AppId
	{0, 0, 0, 0}
};

int parseArgs(int argc, char *argv[], CliArgs_T *CliArgs) {
	int optchar, optindex;
	bool hasRemoteAppId = false,
	     hasRemoteAddr = false;
	while ((optchar = getopt_long(argc, argv, "vhs:p:r:l:", long_options, &optindex)) != -1) {
		switch (optchar) {
			case 'v':
				CliArgs->verbose = true;
				break;
			case 'h':
				ShowUsage();
				exit(EXIT_SUCCESS);
				break;
			case 's':
				CliArgs->SocketPath = optarg;
				break;
			case 'p':
				if (AppIdFromStr(optarg, &CliArgs->RemoteAppId) != FUNC_RESULT_SUCCESS) {
					fprintf(stderr, "ERROR: appid is invalid \"%s\"\n", optarg);
					return FUNC_RESULT_FAILED_ARGUMENT;
				}
				hasRemoteAppId = true;
				break;
			case 'r':
				if (moarAddrFromStr(optarg, &CliArgs->RemoteAddr) != FUNC_RESULT_SUCCESS) {
					fprintf(stderr, "ERROR: remote address is invalid \"%s\"\n", optarg);
					return FUNC_RESULT_FAILED_ARGUMENT;
				}
				hasRemoteAddr = true;
				break;
			case 'l':
				if (AppIdFromStr(optarg, &CliArgs->OwnAppId) != FUNC_RESULT_SUCCESS) {
					fprintf(stderr, "ERROR: appid is invalid: \"%s\"\n", optarg);
					return FUNC_RESULT_FAILED_ARGUMENT;
				}
				CliArgs->listen = true;
				break;
			case '?':
			default:
				return FUNC_RESULT_FAILED_ARGUMENT;
		}
	}
	if (hasRemoteAddr ^ hasRemoteAppId) {
		puts("Application id and remote addr must be set simultaneously!\n");
		ShowUsage();
		exit(EXIT_FAILURE);
	}
	CliArgs->send = (hasRemoteAddr && hasRemoteAppId);
	return FUNC_RESULT_SUCCESS;
}

int main (int argc, char **argv)  {
	CliArgs_T CliArgs = {0};
	srand((unsigned int)time(NULL));
	CliArgs.OwnAppId = (AppId_T)(rand()&0xFFFF);
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
		fprintf(stderr, "Socket successfully bint: descriptor=%d; appid=%d\n", md->SocketFd, CliArgs.OwnAppId);
	
	struct epoll_event events[MAX_EPOLL_EVENTS], event;
	int efd;
	if ((efd = epoll_create(MAX_EPOLL_EVENTS)) == -1) {
		fprintf(stderr, "ERROR: epoll_create failed\n");
		return EXIT_FAILURE;
	}
	
	sockNonBlock(STDIN_FILENO);
	addToEpoll(efd, STDIN_FILENO);
	addToEpoll(efd, md->SocketFd);
	
	ssize_t result = 0;
	while (true) {
		int n = epoll_wait (efd, events, MAX_EPOLL_EVENTS, -1);
		int i;
		for (i = 0; i < n; i++) {
			event = events[i];
//			printf("NEW EVENT: %u [%d %d %d %d]\r\n", events[i].events,
//				   event.events & EPOLLIN,
//				   event.events & EPOLLOUT,
//				   event.events & EPOLLERR,
//				   event.events & EPOLLHUP
//			);
			if (event.data.fd == STDIN_FILENO && CliArgs.send) {
				size_t dataLen = 0;
				MessageStorage_T * storage = MessageStorageCreate(BUFFER_SIZE_INITIAL);
				dataLen = ReadInputData(STDIN_FILENO, storage);
				MessageId_T msgId;
				result = moarSendTo(md, storage->buffer, dataLen, &CliArgs.RemoteAddr, &CliArgs.RemoteAppId, &msgId);
				if (CliArgs.verbose) {
					printf("moarSendTo return value: %zd\n", result);
					fflush(stdout);
				}
				if(result < 0) {
					fprintf(stderr, "ERROR: writing to stack\n");
					return EXIT_FAILURE;
				}
				MessageStorageFree(storage);
			}
			else if (event.data.fd == md->SocketFd) {
				AppId_T RemoteAppId;
				RouteAddr_T RemoteAddr;
				void * inData = NULL;
				result = moarRecvFromRaw(md, &inData, &RemoteAddr, &RemoteAppId);
				if (CliArgs.verbose) {
					printf("moarRecvFromRaw return value: %zd\n", result);
				}
				if(result >=0) {
					OutMessage(inData, (size_t)result);
					fflush(stdout);
					free(inData);
				}
				else {
					fprintf(stderr, "ERROR: reading from stack\n");
					return EXIT_FAILURE;
				}
			}
		}
	}
	moarClose(md);
	return EXIT_SUCCESS;
}
