# MOCK interface tool for MOAR stack #

## About ##

MOCK interface tool (MockIT), also called MOCK physical layer, provides way to transmit data between different interface examplars in a radio waves manner.

## Work logic ##

MockIT is a standalone application for Linux machines, which opens some file socket while running. Other applications ("clients") connect to that socket, register as nodes and then transmit data. MockIT checks whether there are incoming data and proceed it if any by copying from client-sender to all appropriate client-receivers.

## Configuration ##

Configuration for the MockIT should be provided by the suitable configuration file. That file should have the following structure:

* `socket.file` - filename for socket should be specified firstly; no whitespaces are allowed due to reading by `fscanf( "%s", socketFilename )` instruction.
* `123` - frequency in MHz as floating point number; it is assumed, that all nodes in the net use the same frequency specified here.
* `6` - maximum number of nodes in the net; this number should be followed by exactly the same count of lines defining each net node.
* `123 5.31 6.12 1` - line defining current node in the net:
    * `123` - address of the node; should be readable as 4-bytes int;
    * `5.31 6.12` - *X* and *Y* coordinates of the node as floating point numbers; space is assumed to be a 3-dimensional Euclidean, but all nodes are placed on a usual 2-dimensional plane in that space;
    * `1` - sensitivity of the node in dBm as floating point number; minimal power of the signal that can be heared by the node.

The whole file example:

    socket.file
    123
    6
    123 5.31 6.12 1
    5213 512.24 534.21 1
    1 2345 523 1
    2 2.24 53.3 1
    1234 4 2 0.5
    9876 14 42 2.5

## Messages format ##

Every client should work with the MockIT in the following manner:

0. Client should connect to the socket using its name, specified in the first line of the configuration file.
    * Example (for UNIX systems with BSD-version of `netcat`): **`nc -U socket.file`**
    * MockIT will write message like **`2016-07-16 17:38:49.234 : Socket    5 : opened`**

1. Immediately after the connecting client should send a 4-bytes number which should be one of the addresses provided in the configuration file. Address can be specified as a hexademical (starts with `0x`), octal (starts with `0`) or decimal (no special start) integer number.
    * Example: **`5213`**
    * Server will write message like **`2016-07-16 17:40:41.110 : Socket    5 : node 0000145D registered`**

2. Client adds command specifier (special symbol) and appropriate arguments to the socket. MockIT reads data from client and proceeds it according to the command type:
    * **`$`** - sensitivity change; MockIT will read a number right after that symbol and will remember it as new node\`s sensitivity.
        + Example: **`$2`**
        + MockIT will write message like **`2016-07-16 17:41:36.795 : Node 0000145D : new sensitivity : 2.000000`**
    * **`x`**, **`y`** - coordinates change; MockIT will read a number right after that symbol and will remember it as new node\`s coordinate, *X* or *Y*.
        + Example: **`x34`**
        + MockIT will write message like **`2016-07-16 17:43:30.561 : Node 0000145D : new X position : 34.000000`**
    * **`:`** - send message; the very next number is assumed to be an output signal power (on sender), the bytes count **`N`** follows it and yet *N* bytes of the message text.
        + Example: **`:275.5 9 Hi there!`**
        + MockIT will write message like **`2016-07-16 17:48:25.598 : Node 0000145D : - message : 275.500 9 Hi there!`**
        + Nodes which can hear related signal will get message like **`16.707 Hi there!`** , where `16.707` is an input signal power and the message itself goes next.
        + For every node hearing related signal MockIT will write message like **`2016-07-16 17:48:25.598 : Node 000004D2 : + message from node 0000145D`**

3. Client ends work with MockIT just by closing related socket.
    * Example (for UNIX systems with BSD-version of `netcat`): just press **`Ctrl+C`**
    * MockIT will write two messages like **`2016-07-16 17:59:39.804 : Socket    5 : node 0000145D unregistered`** and **`2016-07-16 17:59:39.804 : Socket    5 : closed`**

The whole MockIT output example:

    2016-07-16 17:40:39.353 : Socket    5 : opened
    2016-07-16 17:40:41.110 : Socket    5 : node 0000145D registered
    2016-07-16 17:41:36.795 : Node 0000145D : new sensitivity : 2.000000
    2016-07-16 17:43:30.561 : Node 0000145D : new X position : 34.000000
    2016-07-16 17:43:40.016 : Socket    5 : node 0000145D unregistered
    2016-07-16 17:43:40.016 : Socket    5 : closed
    2016-07-16 17:43:46.145 : Socket    5 : opened
    2016-07-16 17:43:48.915 : Socket    5 : node 0000145D registered
    2016-07-16 17:45:08.560 : Node 0000145D : - message : 275.500 9 Hi there!
    2016-07-16 17:48:15.931 : Socket    6 : opened
    2016-07-16 17:48:17.552 : Socket    6 : node 000004D2 registered
    2016-07-16 17:48:25.598 : Node 0000145D : - message : 275.500 9 Hi there!
    2016-07-16 17:48:25.598 : Node 000004D2 : + message from node 0000145D
    2016-07-16 17:59:39.804 : Socket    5 : node 0000145D unregistered
    2016-07-16 17:59:39.804 : Socket    5 : closed
