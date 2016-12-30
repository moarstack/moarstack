//
// Created by svalov on 12/9/16.
//

#ifndef MOARSTACK_MOARDSETTINGS_H
#define MOARSTACK_MOARDSETTINGS_H

#include <moarSettingsBinder.h>
#include <moarConfigReader.h>

#pragma pack(push, 1)

typedef struct {
	char* LogPath;
	char* LayersEnabledDir;
}moardSettings;

#pragma pack(pop)

int makeMoardSettingsBinding(SettingsBind_T** binding, int* count);
int settingsLoad(moardSettings* settings,  char* fileName, hashTable_T* table);

#endif //MOARSTACK_MOARDSETTINGS_H