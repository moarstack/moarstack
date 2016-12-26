//
// Created by svalov on 12/12/16.
//

#ifndef MOARSTACK_MOCKIFACESETTINGS_H
#define MOARSTACK_MOCKIFACESETTINGS_H

#include <moarSettingsBinder.h>
#include <moarInterface.h>

#pragma pack(push, 1)

typedef struct{
	char		* LogPath,
				* MockItSocket;
	IfaceAddr_T	Address;
}mockIface;

#pragma pack(pop)

int makeMockIfaceBinding(SettingsBind_T** binding, int* count);

#endif //MOARSTACK_MOCKIFACESETTINGS_H
