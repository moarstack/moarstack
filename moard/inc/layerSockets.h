//
// Created by kryvashek on 09.07.16.
//

#ifndef MOARSTACK_LAYERSOCKETS_H
#define MOARSTACK_LAYERSOCKETS_H

#include "moarLibInterface.h"	// MoarLayerType_T
#include "moarChannel.h"		// IfacesCount_T

int socketDown( MoarLayerType_T layerType );
int socketUp( MoarLayerType_T layerType );
int socketsPrepare( IfacesCount_T ifacesCount );

#endif //MOARSTACK_LAYERSOCKETS_H
