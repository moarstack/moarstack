//
// Created by svalov, kryvashek on 05.07.16.
//

#include "moarLayerEntryPoint.h"
#include "moarCommons.h"
#include "moarChannel.h"
#include "moarInterfaceChannel.h"
#include "moarChannelRouting.h"
#include "moarChannelPrivate.h"

void * MOAR_LAYER_ENTRY_POINT(void* arg){
    // load configuration
    // listen for interface connection
    // in poll
        // if new interface connected
            // add to pool list
            // if supported beacon payload
                // update beacon data
                // update beacon data in all interfaces

        // timeout
            //

        // if command from interface

            // connect
                // add interface record to list

            // disconnect
                // remove interface record from list

            // interface/message state
                // change state of interface
                // if sended
                    // send message state command to routing
                // if have more send trys
                    // add to queue
                // else
                    // send message state to routing
                    // drop message
                // if have messages in queue
                        // get message
                    // try to send

            // received
                // if contains neighbors info metadata | hello message
                    // update neighbor information
                // if contains payload
                    // send command to routing
                // else
                    // drop message

            // new neighbor
                // if contains beacon payload
                    //if found neighbor
                        //update neighbor
                    //else
                        // add new neighbor
                        // send to routing new neighbor command
                // else
                    // add to queue hello message

            // lost neighbor
                // if found neighbor
                    // remove interface info
                    // if no interfaces
                        // remove neighbor
                        // send to routing lost neighbor command

            // update neighbor
                // ?????
                // PROFIT

        // if command from routing

            // send
                // if found neighbor
                    // if found free interface
                        // push data to interface
                    // else
                        // if queue is not full
                            // add to queue
                        // else
                            // send error to routing
                // else
                    // send error to routing

}
