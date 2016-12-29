# M.O.A.R. stack #

## General ##

### Name ###

M.O.A.R. - Mesh-Oriented Adaptive Routing, created by **\[censored\] team**. M.O.A.R. stack is a network protocols stack implemeting base ideas of that routing. Names "MOAR stack", "MOARstack" or even just "MOAR" are also available and means "M.O.A.R. stack" in most cases. 

### Description ###

M.O.A.R stack provides way of connecting with unlimited count of devices when no predefined routes and even no special router devices. Actually, M.O.A.R. stack makes a kind of router from any node within the network. Without need in predefined structure, the whole network can easily change its topology, and the only consecuence is a need to restore node routes.

Current implementation of M.O.A.R. stack is made with C99 (ANSI C99) standard of C language and aims Linux systems with kernel of 2.6 version or later.

## Structure ##

### Logical structure ###

M.O.A.R. stack logical structure contains several layers (also called levels):

* Interface - incapsulates routine work for point-to-point connections with nearby nodes.
* Channel - performs choosing the best way to connect to every nearest node and related tasks.
* Routing - layer, which provides routines for finding most appropriate data transfer routes.
* Presentation - layer which implements cryptography, granulating data and gathering it back.
* Service - provides API for other applications to simplify usage of M.O.A.R. stack.

Layers are presented above in the order from the closest to the hardware (interface) to the most abstract (service).

### Directories structure ###

Most of directories in the M.O.A.R. stack repository have their own purposes:

* `./` - in the root directory files (CMakeLists.txt actually) to build the entire project and all the other project subdirectories (sometimes gathered in one subdirectory, as it is for layers) are presented.
* `layers` - directories with headers and sources for every layer, including common ones for all layers.
    * `interface`, `channel`, `routing`, `presentation`, `service` - directories with sources and headers only related to one layer per folder.
    * `shared` - files (mostly headers) which are common for all layers.
* `libmoar` - sources and headers, which can be used in other projects to connect with M.O.A.R. stack and use it to transfer data; thus, these files implement API for M.O.A.R. stack (point of connection with service layer).
* `moard` - sources and headers, gathering all the layers into the linux daemon.
* `tools` - sources and headers of additional service programms using M.O.A.R. stack or to be used with M.O.A.R. stack.

## Tools ##

There are no tools implemented yet, but creating of at least one is planned:

### Ping ###

Program to check accessibility of node specified by its address. Sends special packets with some interval, receives replies and gathers statistics about delivering these packets.

## Copyright ##

### License ###

Current license for M.O.A.R. stack is BSD-3-Clause

### Copyright holder ###

No matter of license type, author of MOAR is **\[censored\] team**, and rights to decide what to do with project and which license to use belongs to the **\[censored\] team**.
If we will not like what you do with this software - we will find you, sue you and we will piss on your body afterwards.