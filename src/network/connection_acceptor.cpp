#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include "connection_acceptor.h"

ConnectionAcceptor::ConnectionAcceptor(types::port_t port, int backlog_size) {
    int err;
    struct sockaddr_in6 serveraddr;

    // Support for both IPv4 and IPv6 addresses.
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin6_family = AF_INET6;
    serveraddr.sin6_port = htons(port);
    serveraddr.sin6_addr = in6addr_any;

    // Create a new TCP socket.
    socket_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        throw TCPAcceptError(std::strerror(errno));
    }

    // Bind the socket to the specified port.
    err = bind(socket_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if (err != 0) {
        throw TCPAcceptError(gai_strerror(err));
    }

    // Start listening for new connection requests.
    err = listen(socket_fd, backlog_size);
    if (err != 0) {
        throw TCPAcceptError(gai_strerror(err));
    }
}

int ConnectionAcceptor::accept_another_connection() const {
    int new_connection_fd = accept(socket_fd, nullptr, nullptr);
    if (new_connection_fd == -1) {
        throw TCPAcceptError(std::strerror(errno));
    }

    // Disable Nagle's algorithm.
    int err;
    int flag = 1;
    err = setsockopt(new_connection_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    if (err != 0) {
        throw std::runtime_error(std::strerror(errno));
    }

    return new_connection_fd;
}

ConnectionAcceptor::~ConnectionAcceptor() {
    if (close(socket_fd) == -1) {
        std::cerr << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}