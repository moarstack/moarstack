//
// Created by kryvashek on 09.07.16.
//

#ifndef MOARSTACK_LAYERSOCKETS_H
#define MOARSTACK_LAYERSOCKETS_H

#include <moarLibInterface.h>	// MoarLayerType_T
#include <moarCommons.h>		// SocketFilepath_T

int SocketDown( MoarLayerType_T layerType );
int SocketUp( MoarLayerType_T layerType );
int SocketsPrepare( const SocketFilepath_T ifaceSocketFilePath, const SocketFilepath_T serviceSocketFilePath  );

#endif //MOARSTACK_LAYERSOCKETS_H
