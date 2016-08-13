//
// Created by svalov on 8/13/16.
//

#ifndef MOARSTACK_MOARROUITNGCOMMANDPROCESSING_H
#define MOARSTACK_MOARROUITNGCOMMANDPROCESSING_H

int processChannelEvent(RoutingLayer_T* layer, int fd, uint32_t event);
int processPresentationEvent(RoutingLayer_T* layer, int fd, uint32_t event);

#endif //MOARSTACK_MOARROUITNGCOMMANDPROCESSING_H
