#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "messages.h"

// Client-specific.

/**
 * @brief Read server's message sent over TCP connection.
 *
 * @param socket TCP socket file descriptor.
 * @return server's message.
 */
ServerMessage read_client_server_message(int socket);

/**
 * @brief Send message to server over TCP connection.
 *
 * @param message Message to be sent.
 * @param socket TCP socket file descriptor.
 */
void send_client_server_message(ClientMessage& message, int socket);

/**
 * @brief Read GUI's message sent over UDP connection.
 *
 * @param socket UDP socket file descriptor.
 */
InputMessage read_client_gui_message(int socket);

/**
 * @brief Read GUI's message sent over UDP connection.
 *
 * @param socket UDP socket file descriptor. The socket should be connected with GUI.
 */
void send_client_gui_message(DrawMessage& message, int socket);



#endif // SERIALIZER_H