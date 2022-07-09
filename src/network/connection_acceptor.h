
/**
 * @author Olaf Placha
 * @brief This module provides a class used for accepting clients' TCP connection requests.
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef CONNECTION_ACCEPTOR_H
#define CONNECTION_ACCEPTOR_H

#include <stdexcept>
#include "../config/config.h"

class TCPAcceptError : public std::runtime_error {
public:
    explicit TCPAcceptError(const char *w) : std::runtime_error(w) {}
};

class ConnectionAcceptor {
public:
    ConnectionAcceptor(types::port_t port, int backlog_size);

    /**
     * @brief Accepts another TCP connection. Turns off Nagle's congestion algorithm.
     * 
     * @throw TCPAcceptError - Thrown when any network related system call fails.
     * @return int - File descriptor of the socket of the newly established TCP connection.
     */
    [[nodiscard]] int accept_another_connection() const;

    ~ConnectionAcceptor();

    /* Delete copy constructor and copy assignment. */
    ConnectionAcceptor(ConnectionAcceptor const &) = delete;

    void operator=(ConnectionAcceptor const &) = delete;

private:
    int socket_fd;
};

#endif // CONNECTION_ACCEPTOR_H