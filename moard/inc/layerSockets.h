//
// Created by kryvashek on 09.07.16.
//

#ifndef MOARSTACK_LAYERSOCKETS_H
#define MOARSTACK_LAYERSOCKETS_H

#include <moarLibInterface.h>	// MoarLayerType_T
#include <moarCommons.h>		// SocketFilepath_T

__BEGIN_DECLS

extern int SocketDown( MoarLayerType_T layerType );
extern int SocketUp( MoarLayerType_T layerType );
extern int SocketsPrepare( const SocketFilepath_T ifaceSocketFilePath, const SocketFilepath_T serviceSocketFilePath  );

__END_DECLS

#endif //MOARSTACK_LAYERSOCKETS_H
