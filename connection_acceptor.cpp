#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include "connection_acceptor.h"

ConnectionAcceptor::ConnectionAcceptor(types::port_t port, int backlog_size)
{
    int err;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve my address.
    err = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        throw std::runtime_error(gai_strerror(err));
    }

    // Create a new TCP socket.
    socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_fd == -1) {
        throw std::runtime_error(std::strerror(errno));
    }

    // Bind the socket to the specified port.
    err = bind(socket_fd, res->ai_addr, res->ai_addrlen);
    if (err != 0) {
        throw std::runtime_error(gai_strerror(err));
    }

    // Start listening for new connection requests.
    err = listen(socket_fd, backlog_size);
    if (err != 0) {
        throw std::runtime_error(gai_strerror(err));
    }
}

int ConnectionAcceptor::accept_another_connection()
{
    int new_connection_fd = accept(socket_fd, NULL, NULL);
    if (new_connection_fd == -1) {
        throw std::runtime_error(std::strerror(errno));
    }

    return new_connection_fd;
}

ConnectionAcceptor::~ConnectionAcceptor()
{
    if(close(socket_fd) == -1) {
        std::cerr << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}