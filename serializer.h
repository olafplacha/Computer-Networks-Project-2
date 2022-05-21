#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "messages.h"

// Client-specific.

/**
 * Returns server's message sent over TCP connection.
 *
 * @param socket Socket file descriptor.
 * @return server's message.
 */
ServerMessage readServerMessage(int socket);

#endif // SERIALIZER_H