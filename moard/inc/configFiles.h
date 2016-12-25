//
// Created by svalov on 7/4/16.
//

#ifndef MOARSTACK_CONFIGFILES_H
#define MOARSTACK_CONFIGFILES_H

#ifdef USE_CLION_PATH
//clion files
#define LIBRARY_PATH_INTERFACE "../layers/interface/libmoar_interface.so";
#define LIBRARY_PATH_CHANNEL "../layers/channel/libmoar_channel.so";
#define LIBRARY_PATH_ROUTING "../layers/routing/libmoar_routing.so";
#define LIBRARY_PATH_PRESENTATION "../layers/presentation/libmoar_presentation.so";
#define LIBRARY_PATH_SERVICE "../layers/service/libmoar_service.so";
#define CONFIG_FILE					"../config/moarstack/moar.conf"
#else
//clion files
#define LIBRARY_PATH_INTERFACE		"../lib/libmoar_interface.so";
#define LIBRARY_PATH_CHANNEL		"../lib/libmoar_channel.so";
#define LIBRARY_PATH_ROUTING		"../lib/libmoar_routing.so";
#define LIBRARY_PATH_PRESENTATION	"../lib/libmoar_presentation.so";
#define LIBRARY_PATH_SERVICE		"../lib/libmoar_service.so";
#define CONFIG_FILE					"../config/moarstack/moar.conf"
//#define CONFIG_FILE				"/etc/moarstack/moar.conf"
#endif

#endif //MOARSTACK_CONFIGFILES_H
