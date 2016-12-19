//
// Created by svalov on 12/10/16.
//

#ifndef MOARSTACK_MOARCOMMONSETTINGS_H
#define MOARSTACK_MOARCOMMONSETTINGS_H

#include <moarSettingsBinder.h>
#include <moarRouting.h>
#include <moarChannel.h>

// node address
typedef struct{
	RouteAddr_T Address;
} nodeAddress;
// interface socket
typedef struct{
	char* FileName;
} ifaceSocket;
// service socket
typedef struct{
	char* FileName;
} serviceSocket;
typedef struct{
	char* FileName;
} libraryLocation;

__BEGIN_DECLS
extern int makeAddressBinding(SettingsBind_T** binding, int* count);
extern int makeIfaceSockBinding(SettingsBind_T** binding, int* count);
extern int makeServSockBinding(SettingsBind_T** binding, int* count);
extern int makeLibraryLocationBinding(SettingsBind_T** binding, int* count);
__END_DECLS

#endif //MOARSTACK_MOARCOMMONSETTINGS_H
