//
// Created by svalov, kryvashek on 05.07.16.
//

#include "moarLayerEntryPoint.h"
#include "moarCommons.h"
#include "moarInterface.h"
#include "moarInterfaceChannel.h"
#include "moarInterfacePrivate.h"

void * MOAR_LAYER_ENTRY_POINT(void* arg){
    // load configuration
    // connect to channel layer
    // send connect command
    // wait for connected answer
    // prepare physical interface
    // in poll
        // if timeout
            // if time to send beacon | interface ready
                // send beacon

        // if signal event
            // ?????

        // if transmission done
            // switch interface to listen
            // update current message state

        // if new data received
            // if correct crc
                // if data need response
                    // send response
                // if data is response
                    // update current message state
                    // drop message
                // if data is beacon
                    // if neighbor found
                        //update neighbor
                    // else
                        // add neighbor
                        // send to channel new neighbor command
                // if contains payload

        // socket have data
            // if interface ready
                //read command
                //commands
                    // send
                        // if neighbor not found
                            // send error
                        // send message
                        // update current message state
                    // update beacon payload
                        // update stored beacon payload
}