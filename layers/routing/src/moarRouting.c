//
// Created by svalov, kryvashek on 05.07.16.
//

#include "moarLayerEntryPoint.h"
#include "moarCommons.h"
#include "moarRouting.h"
#include "moarChannelRouting.h"
#include "moarRoutingPresentation.h"

void * MOAR_LAYER_ENTRY_POINT(void* arg){
    // load configuration
    // in poll
        // commands
            // send
                // read message and store in queue
            // message state
                // if message sended
                    // send message state update to presentation
                    // drop message
                // if not sended
                    // if no trys
                        // send message state update to presentation
                    // else
                        // add to routing queue

            // new neighbor
                // add channel layer neighbor to routing
            // lost neighbor
                // remove channel layer neighbor to routing
            // update neighbor
                // ?????
        //timeout | end of command processing
            // if need to send probes
                // add probe to queue | send probe to channel layer
            // try to process message queue
            // calculate optimal sleep time
            // change pool timeout

}