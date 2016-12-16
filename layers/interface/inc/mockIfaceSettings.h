//
// Created by svalov on 12/12/16.
//

#ifndef MOARSTACK_MOCKIFACESETTINGS_H
#define MOARSTACK_MOCKIFACESETTINGS_H

#include <moarSettingsBinder.h>
#include <moarInterface.h>

typedef struct{
	char		* LogPath,
				* MockItSocket;
	IfaceAddr_T	Address;
}mockIface;

int makeMockIfaceBinding(SettingsBind_T** binding, int* count);

#endif //MOARSTACK_MOCKIFACESETTINGS_H
