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

License for M.O.A.R. stack is not specified yet, but this issue is under active work. Next points should be accomplished with license for M.O.A.R. stack:

1. MOAR is provided "as is", without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose and noninfringement; in no event shall the authors or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or the use or other dealings in the software.
2. Permission to compose MOAR or any use with(in) proprietary projects (for example, to use proprietary WiFi drivers and so on).
3. Permission to compose open source projects with MOAR (for example, to include MOAR into Linux kernel).
4. Permission to run derivative proprietary or open source projects by composing them with MOAR or any other kind of using.
5. Permission to compose(or any other usage of) MOAR with any derivative project with licensing MOAR under the same (as current) license with no breaking of derivative product license.
6. In order to keep compatibility of points 4 and 5 derivative product should have notification that it is composed with MOAR and about the related MOAR license (including the version information, both for MOAR and its license).

NOTE: "To compose" in above text means "to use by linkage with - whether static or dynamic - or by copying and possible modifying of the source code - whether partially or in the whole".


### Copyright holder ###

No matter of license type, author of MOAR is **\[censored\] team**, and rights to decide what to do with project and which license to use belongs to the **\[censored\] team**.
If we will not like what you do with this software - we will find you, sue you and we will piss on your body afterwards.