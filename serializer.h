#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "messages.h"

// Client-specific.

/**
 * Read server's message sent over TCP connection.
 *
 * @param socket TCP socket file descriptor.
 * @return server's message.
 */
ServerMessage readClientServerMessage(int socket);

/**
 * Send message to server over TCP connection.
 *
 * @param message Message to be sent.
 * @param socket TCP socket file descriptor.
 */
void sendClientServerMessage(ClientMessage& message, int socket);

/**
 * Read GUI's message sent over UDP connection.
 *
 * @param socket UDP socket file descriptor.
 */
InputMessage readClientGUIMessage(int socket);

/**
 * Read GUI's message sent over UDP connection.
 *
 * @param socket UDP socket file descriptor. The socket should be connected with GUI.
 */
void sendClientGUIMessage(DrawMessage& message, int socket);



#endif // SERIALIZER_H