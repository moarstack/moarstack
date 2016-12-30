//
// Created by svalov on 12/12/16.
//

#ifndef MOARSTACK_MOCKIFACESETTINGS_H
#define MOARSTACK_MOCKIFACESETTINGS_H


#include <moarSettingsBinder.h>
#include <moarInterface.h>
#include <moarInterfacePrivate.h>

#define DEFAULT_MOCKIFACE_LOG_FILE	"/tmp/moarMockIface.log"
#define DEFAULT_MOCKIFACE_ADDRESS	((IFACE_ADDR_BASE_TYPE)1)
#define DEFAULT_MOCKIT_SOCKET_FILE	"/tmp/moarMockIt.sock"
#define DEFAULT_CHANNEL_SOCKET_FILE	"/tmp/moarIfaceChannel.sock"


#pragma pack(push, 1)

typedef struct{
	char			* LogPath,
					* MockItSocket;
	IfaceAddr_T		Address;
	PowerFloat_T	Sensitivity;
}mockIface;

#pragma pack(pop)

int makeMockIfaceBinding(SettingsBind_T** binding, int* count);

#endif //MOARSTACK_MOCKIFACESETTINGS_H
