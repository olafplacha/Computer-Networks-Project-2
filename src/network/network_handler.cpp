#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
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
            throw std::invalid_argument("Invalid data type size for endianness convertion!");
    }
}

/**
 * @brief Converts first n bytes of the buffer from host to network byte order.
 *
 * @param buffer Pointer to buffer with data.
 * @param n Number of bytes to convert.
 */
void convert_host_to_network_byte_order(uint8_t* buffer, size_t n)
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
            throw std::invalid_argument("Invalid data type size for endianness convertion!");
    }
}

uint8_t* NetworkHandler::allocate_buffer_space(size_t n)
{
    uint8_t* buff_ptr = (uint8_t *) malloc(n);
    if (buff_ptr == NULL) {
        throw std::runtime_error("Error occured when allocating space for a buffer!");
    }
    return buff_ptr;
}

NetworkHandler::NetworkHandler(size_t recv_buff_size_, size_t send_buff_size_) :
        recv_buff_size(recv_buff_size_), send_buff_size(send_buff_size_)
{
    recv_buff = allocate_buffer_space(recv_buff_size_);
    send_buff = allocate_buffer_space(send_buff_size_);
}

NetworkHandler::~NetworkHandler()
{
    free(recv_buff);
    free(send_buff);
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
        throw std::runtime_error(gai_strerror(err));
    }

    // Create a TCP socket.
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        throw std::runtime_error(std::strerror(errno));
    }

    // Connect to the server.
    err = connect(fd, res->ai_addr, res->ai_addrlen);
    if (err != 0) {
        throw std::runtime_error(std::strerror(errno));
    }

    // Disable Nagle's algorithm.
    int flag = 1;
    err = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    if (err != 0) {
        throw std::runtime_error(std::strerror(errno));
    }

    freeaddrinfo(res);
    return fd;
}

TCPHandler::TCPHandler(int socket_fd_, size_t buff_size_) :
        NetworkHandler(buff_size_, buff_size_), socket_fd(socket_fd_), recv_deque() {}

TCPHandler::TCPHandler(std::string& address, types::port_t port, size_t buff_size_) :
        NetworkHandler(buff_size_, buff_size_), recv_deque()
{
    socket_fd = set_up_tcp_connection(address, port);
}

TCPHandler::~TCPHandler()
{
    if (shutdown(socket_fd, SHUT_WR) == -1) {
        // Ignore errors.
    }
    if(close(socket_fd) == -1) {
        std::cerr << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void TCPHandler::return_when_n_bytes_in_deque(size_t n)
{
    // If there are enough bytes in the recv_deque, then do not read on the socket.
    while (recv_deque.size() < n) {
        ssize_t received_bytes = recv(socket_fd, recv_buff, recv_buff_size, 0);
        if (received_bytes == 0) {
            throw TCPError("Peer disconnected!");
        }
        else if (received_bytes < 0) {
            // Some error occured.
            throw TCPError(std::strerror(errno));
        }
        // Copy all read bytes to recv_deque.
        for (ssize_t i = 0; i < received_bytes; i++)
        {
            recv_deque.push_back(recv_buff[i]);
        }
    }
}

void TCPHandler::send_n_bytes(size_t n, uint8_t* buff)
{
    // Send until there are no bytes to be sent.
    while (n > 0) {
        ssize_t bytes_sent = send(socket_fd, buff, n, MSG_NOSIGNAL);
        if (bytes_sent == -1) {
            // Some error occured.
            throw TCPError(std::strerror(errno));
        }
        n -= bytes_sent;
        buff += bytes_sent;
    }
}

int UDPHandler::set_up_udp_listening(types::port_t port)
{
    int fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (fd < 0) {
        throw std::runtime_error(std::strerror(errno));
    }

    struct sockaddr_in6 address;
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(port);

    // Bind port to the socket.
    if(bind(fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        throw std::runtime_error(std::strerror(errno));
    }
    return fd;
}

int UDPHandler::set_up_udp_sending(std::string address, types::port_t port)
{
    int err;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // Resolve the address.
    err = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        throw std::runtime_error(gai_strerror(err));
    }

    // Create a UDP socket.
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        throw std::runtime_error(std::strerror(errno));
    }

    // Bind the packets receiver.
    err = connect(fd, res->ai_addr, res->ai_addrlen);
    if (err != 0) {
        throw std::runtime_error(std::strerror(errno));
    }

    freeaddrinfo(res);
    return fd;
}

UDPHandler::UDPHandler(types::port_t recv_port, std::string send_address, types::port_t send_port,
                       size_t buff_size_) : NetworkHandler(buff_size_, buff_size_), packet_size(0)
{
    recv_socket_fd = set_up_udp_listening(recv_port);
    send_socket_fd = set_up_udp_sending(send_address, send_port);
    recv_pointer = recv_buff;
    send_pointer = send_buff;
}

size_t UDPHandler::read_incoming_packet()
{
    // Reset the recv_pointer.
    recv_pointer = recv_buff;

    // Read another UDP packet.
    ssize_t bytes_read = recv(recv_socket_fd, recv_buff, recv_buff_size, 0);
    if (bytes_read < 0) {
        // Some error occured.
        throw UDPError(std::strerror(errno));
    }

    packet_size = bytes_read;
    return packet_size;
}

void UDPHandler::flush_outcoming_packet()
{
    size_t bytes_to_send = send_pointer - send_buff;
    ssize_t bytes_sent = send(send_socket_fd, send_buff, bytes_to_send, 0);
    if (bytes_sent < 0) {
        // Some error occured.
        throw UDPError(std::strerror(errno));
    }

    // Free the buffer for next UDP packet.
    send_pointer = send_buff;
}

UDPHandler::~UDPHandler()
{
    if(close(recv_socket_fd) == -1) {
        std::cerr << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    if(close(send_socket_fd) == -1) {
        std::cerr << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::string TCPHandler::get_peer_name() const
{
    int err;
    char str[INET6_ADDRSTRLEN];
    struct sockaddr_in6 address;
    socklen_t addres_len = sizeof(address);

    err = getpeername(socket_fd, (struct sockaddr *) &address, &addres_len);
    if (err == -1) {
        throw std::runtime_error(std::strerror(errno));
    }

    if(inet_ntop(AF_INET6, &address.sin6_addr, str, sizeof(str))) {
        return "[" + std::string(str) + "]:" + std::to_string(ntohs(address.sin6_port));
    }
    else {
        throw std::runtime_error(std::strerror(errno));
    }
}