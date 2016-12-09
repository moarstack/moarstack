//
// Created by svalov on 12/9/16.
//

#ifndef MOARSTACK_MOARDSETTINGS_H
#define MOARSTACK_MOARDSETTINGS_H

#include <moarSettingsBinder.h>
#include <moarConfigReader.h>

typedef struct {
	char* LogPath;
	char* LayersEnabledDir;
}moardSettings;

int makeMoardSettingsBinding(SettingsBind_T** binding, int* count);

#endif //MOARSTACK_MOARDSETTINGS_H