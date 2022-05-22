#include <iostream>
#include <inttypes.h>
#include <arpa/inet.h>
#include "serializer.h"
#include "config.h"

void check_connection(bool connected)
{
    if (!connected) {
        std::cerr << "Server disconnected!\n";
        exit(EXIT_FAILURE);
    }
}

Hello read_hello_message(TCPHandler& handler)
{
    
}

ServerMessage read_client_server_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];

    bool connected_to_server = handler.read_n_bytes(sizeof(types::message_id_t), buffer);
    check_connection(connected_to_server);

    types::message_id_t message_id = *(types::message_id_t *) buffer;

    switch (message_id)
    {
    case 0:
        
        break;
    case 1:

        break;
    case 2:

        break;

    case 3:

        break;

    case 4:

        break;
    default:
        // Wrong message id.
        break;
    }
}
