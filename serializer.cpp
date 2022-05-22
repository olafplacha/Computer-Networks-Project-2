#include <iostream>
#include <inttypes.h>
#include <arpa/inet.h>
#include "serializer.h"
#include "config.h"

/**
 * @brief Converts n first bytes of the buffer from network to host byte order.
 * 
 * @param buffer Pointer to buffer with data.
 * @param n Number of bytes to convert.
 */
void convert_network_to_host_byte_order(uint8_t* buffer, size_t n)
{
    switch (n)
    {
    case 1:
        // Nothing to be done.
        return;   
    case 2:
    {
        uint16_t before = *(uint16_t *) buffer;
        uint16_t after = ntohs(before);
        *(uint16_t *) buffer = after;
        return;
    }
    case 4:
    {
        uint32_t before = *(uint32_t *) buffer;
        uint32_t after = ntohl(before);
        *(uint32_t *) buffer = after;
        return;
    }
    case 8:
    {
        uint64_t before = *(uint64_t *) buffer;
        uint64_t after = be64toh(before);
        *(uint64_t *) buffer = after;
        return;
    }
    default:
        // Unreachable.
        return;
    }
}

ServerMessage read_client_server_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];
    bool connected_to_server;

    connected_to_server = handler.read_n_bytes(sizeof(types::message_id_t), buffer);

}
