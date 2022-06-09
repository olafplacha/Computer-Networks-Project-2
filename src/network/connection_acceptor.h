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
     * @brief Accept another TCP connection. Turn off Nagle's congestion algorithm.
     * 
     * @return int Socket of the new TCP connection.
     */
    [[nodiscard]] int accept_another_connection() const;

    ~ConnectionAcceptor();

    // Delete copy constructor and copy assignment.
    ConnectionAcceptor(ConnectionAcceptor const &) = delete;

    void operator=(ConnectionAcceptor const &) = delete;

private:
    int socket_fd;
};

#endif // CONNECTION_ACCEPTOR_H