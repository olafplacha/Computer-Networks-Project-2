#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include "network_handler.h"

/**
 * @brief Converts first n bytes of the buffer from network to host byte order.
 * 
 * @param buffer Pointer to buffer with data.
 * @param n Number of bytes to convert.
 */
static void convert_network_to_host_byte_order(uint8_t* buffer, size_t n)
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
        throw std::invalid_argument("invalid data type size");
    }
}

/**
 * @brief Converts first n bytes of the buffer from host to network byte order.
 * 
 * @param buffer Pointer to buffer with data.
 * @param n Number of bytes to convert.
 */
static void convert_host_to_network_byte_order(uint8_t* buffer, size_t n)
{
    switch (n)
    {
    case 1:
        // Nothing to be done.
        return;   
    case 2:
    {
        uint16_t before = *(uint16_t *) buffer;
        uint16_t after = htons(before);
        *(uint16_t *) buffer = after;
        return;
    }
    case 4:
    {
        uint32_t before = *(uint32_t *) buffer;
        uint32_t after = htonl(before);
        *(uint32_t *) buffer = after;
        return;
    }
    case 8:
    {
        uint64_t before = *(uint64_t *) buffer;
        uint64_t after = htobe64(before);
        *(uint64_t *) buffer = after;
        return;
    }
    default:
        throw std::invalid_argument("invalid data type size");
    }
}

int TCPHandler::set_up_tcp_connection(std::string& address, types::port_t port)
{
    int err;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the address.
    err = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        std::cerr << gai_strerror(err) << '\n';
        exit(EXIT_FAILURE);
    }

    // Create a TCP socket.
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        std::cerr << std::strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }

    // Connect to the server.
    err = connect(fd, res->ai_addr, res->ai_addrlen);
    if (err != 0) {
        std::cerr << std::strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }

    // Disable Nagle's algorithm.
    int flag = 1;
    err = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    if (err < 0) {
        std::cerr << std::strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    return fd;
}

uint8_t* TCPHandler::allocate_buffer_space(size_t n)
{
    uint8_t* buff_ptr = (uint8_t *) malloc(n);
    if (buff_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return buff_ptr;
}

TCPHandler::TCPHandler(int socket_fd_, size_t recv_buff_size_) : socket_fd(socket_fd_), 
    recv_buff_size(recv_buff_size_), recv_deque()
{
    recv_buff = allocate_buffer_space(recv_buff_size_);
}

TCPHandler::TCPHandler(std::string& address, types::port_t port, size_t recv_buff_size_) :
    recv_buff_size(recv_buff_size_)
{
    socket_fd = set_up_tcp_connection(address, port);
    recv_buff = allocate_buffer_space(recv_buff_size_);
}

TCPHandler::~TCPHandler()
{
    free(recv_buff);
    if(close(socket_fd) == -1) {
        std::cerr << std::strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }
}

bool TCPHandler::return_when_n_bytes_in_deque(size_t n)
{
    // If there are enough bytes in the recv_deque, then do not read on the socket.
    while (recv_deque.size() < n) {
        ssize_t received_bytes = recv(socket_fd, recv_buff, recv_buff_size, 0);
        // Check if read successful.
        if (received_bytes < 0) {
            std::cerr << std::strerror(errno) << '\n';
            exit(EXIT_FAILURE);
        }
        // Check if peer disconnected.
        if (received_bytes == 0) {
            return false;
        }
        // Copy all read bytes to recv_deque.
        for (ssize_t i = 0; i < received_bytes; i++)
        {
            recv_deque.push_back(recv_buff[i]);
        }
    }
    return true;
}

bool TCPHandler::read_n_bytes(size_t n, uint8_t* buff)
{
    bool connected = return_when_n_bytes_in_deque(n);
    if (!connected) {
        return false;
    }
    // At this point there are at least n bytes in the deque.
    for (size_t i = 0; i < n; i++)
    {
        buff[i] = recv_deque.front();
        recv_deque.pop_front();
    }
    // Convert the endianness if needed.
    convert_network_to_host_byte_order(buff, n);
    return true;
}

bool TCPHandler::send_n_bytes(size_t n, uint8_t* buff)
{
    // Convert the endianness if needed.
    convert_host_to_network_byte_order(buff, n);

    // Send until there are no bytes to be sent.
    while (n > 0) {
        ssize_t bytes_sent = send(socket_fd, buff, n, 0);
        if (bytes_sent == -1) {
            // Some error occured.
            if (errno == ECONNRESET) {
                // Connection reset by peer.
                return false;
            }
            else {
                std::cerr << std::strerror(errno) << '\n';
                exit(EXIT_FAILURE);
            }
        }
        n -= bytes_sent;
        buff += bytes_sent;
    }
    return true;
}