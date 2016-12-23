#ifndef __MOAR_API_H__
#define __MOAR_API_H__

#include "moarApiCommon.h"

__BEGIN_DECLS
/**
 * Utility function. Gets address from its string representation.
 * @param address Node address in hexademical string form
 * @param routeAddr Node address in MOAR form
 * @return Execution result, 0 if success, negative if failed
 */
extern int moarAddrFromStr(char* address, RouteAddr_T* routeAddr);

/**
 * Closes connection, releases used memory, frees descriptor, drops binding from socket and application id.
 * Also binding will be dropped after application terminated
 * @param fd Descriptor
 * @return Execution result, 0 if success, negative if failed
 */
extern int moarClose(MoarDesc_T *fd);

/**
 * Utility function. Gets internal connection descriptor.
 * Descriptor can be used for standart functions like poll, select and etc.
 * @param fd Pointer to descriptor
 * @return Execution result, greater or equal to 0 if success, negative if failed
 */
extern int moarSocketGetDescriptor(MoarDesc_T *fd);

/**
 * Generates socket descriptor.
 * @return New socket descriptor, NULL if failed
 */
extern MoarDesc_T* moarSocket();
/**
 * Generate socket descriptor with specified stack socket file.
 * Should be used for connecting to diffetent stacks running on single computer
 * @param fileName File name of socket
 * @return  New socket descriptor, NULL if failed
 */
extern MoarDesc_T* moarSocketFile(char* fileName);
/**
 * Binds application and socket. Meaning is similar to standard UDP socket.
 * @param fd Pointer to descriptor.
 * @param appId Application id.
 * @return Execution result, 0 if success, negative if failed
 */
extern int moarBind(MoarDesc_T* fd, const AppId_T *appId);


/**
 * Traditional receive function. Places data to msg limited by size len
 * Function blocks thread until data will be received
 * @param fd Socket descriptor
 * @param msg Message
 * @param len Maximum message length. If received data amount is greater that this parameter, all excessive data will be dropped.
 * @param routeAddr Return param, sender address.
 * @param appId Return param, sender application id.
 * @return Amount of data received. Negative value means no data was received.
 */
extern ssize_t moarRecvFrom(MoarDesc_T* fd, void *msg, size_t len, RouteAddr_T *routeAddr, AppId_T  *appId);


/**
 * Raw variant of Read. Return allocated message buffer.
 * Function blocks thread until data will be received
 * @param fd Socket descriptor
 * @param msg Message
 * @param routeAddr Return param, sender address.
 * @param appId Return param, sender application id.
 * @return Amount of data received. Negative value means no data was received.
 */
extern ssize_t moarRecvFromRaw(MoarDesc_T* fd, void **msg, RouteAddr_T *routeAddr, AppId_T  *appId);

/**
 * Traditional send function. Sends msg data to remote node application
 * @param fd Socket descriptor
 * @param msg Message
 * @param len Message length.
 * @param routeAddr Destination address
 * @param appId Remote port application descriptor
 * @param msgId Return param, message id assigned by MOAR. Can be set to NULL, if message id not needed.
 * @return Amount of data transmitted. Must be equal to len parameter.
 */
extern ssize_t moarSendTo(MoarDesc_T* fd, const void *msg, size_t len, const RouteAddr_T *routeAddr, const AppId_T *appId, MessageId_T *msgId);

/**
 * Gets current message delivery state
 * @param fd Socket descriptor
 * @param msgId Message Id. Could be obtained from moarSendTo function.
 * @param state Return param, message delivery state
 * @return Execution result, 0 if success, negative if failed
 */
extern int moarMsgState(MoarDesc_T* fd, const MessageId_T *msgId, MessageState_T *state);


__END_DECLS

#endif
