//
// Created by kryvashek on 09.07.16.
//

#ifndef MOARSTACK_LAYERSOCKETS_H
#define MOARSTACK_LAYERSOCKETS_H

#include "moarLibInterface.h"	// MoarLayerType_T
#include "moarChannel.h"		// IfacesCount_T

int socketDown( MoarLayerType_T layerType, IfacesCount_T ifaceIndex );
int socketUp( MoarLayerType_T layerType, IfacesCount_T ifaceIndex );
int socketsPrepare( IfacesCount_T ifacesCount );

#endif //MOARSTACK_LAYERSOCKETS_H
