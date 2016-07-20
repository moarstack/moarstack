//
// Created by kryvashek on 09.07.16.
//

#ifndef MOARSTACK_LAYERSOCKETS_H
#define MOARSTACK_LAYERSOCKETS_H

#include "moarLibInterface.h"	// MoarLayerType_T
#include "moarCommons.h"		// SocketFilepath_T


int socketDown( MoarLayerType_T layerType );
int socketUp( MoarLayerType_T layerType );
int socketOpenFile( const SocketFilepath_T socketFilePath );
int socketsPrepare( const SocketFilepath_T ifaceSocketFilePath, const SocketFilepath_T serviceSocketFilePath  );

#endif //MOARSTACK_LAYERSOCKETS_H
