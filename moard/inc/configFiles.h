//
// Created by svalov on 7/4/16.
//

#ifndef MOARSTACK_CONFIGFILES_H
#define MOARSTACK_CONFIGFILES_H

#define USE_CLION

#ifdef USE_CLION
//clion files
#define LIBRARY_PATH_INTERFACE "../layers/interface/libmoar_interface.so";
#define LIBRARY_PATH_CHANNEL "../layers/channel/libmoar_channel.so";
#define LIBRARY_PATH_ROUTING "../layers/routing/libmoar_routing.so";
#define LIBRARY_PATH_PRESENTATION "../layers/presentation/libmoar_presentation.so";
#define LIBRARY_PATH_SERVICE "../layers/service/libmoar_service.so";
#else
//clion files
#define LIBRARY_PATH_INTERFACE "../lib/libmoar_interface.so";
#define LIBRARY_PATH_CHANNEL "../lib/libmoar_channel.so";
#define LIBRARY_PATH_ROUTING "../lib/libmoar_routing.so";
#define LIBRARY_PATH_PRESENTATION "../lib/libmoar_presentation.so";
#define LIBRARY_PATH_SERVICE "../lib/libmoar_service.so";
#endif

#endif //MOARSTACK_CONFIGFILES_H
