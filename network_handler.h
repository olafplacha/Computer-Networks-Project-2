#ifndef NETWORK_HANDLER_H
#define NETWORK_HANDLER_H

#include <deque>
#include <string>
#include <inttypes.h>
#include <stdexcept>
#include "config.h"

class TCPError: public std::runtime_error {
    public:
        TCPError(const char* w) : std::runtime_error(w) {}
};

/**
 * @brief Class wrapping reading and writing on a TCP socket. Objects of this class can be
 * instantiated providing previously created socket or by providing name and port of the 
 * server, with which connection will be established during object construction.
 * 
 */
class TCPHandler {
    public:
        /**
         * @brief Construct a new TCPHandler object using previously created socket.
         * 
         * @param socket_fd_ Connected TCP socket file descriptor.
         * @param recv_buff_size_ Size of the receive buffer.
         */
        TCPHandler(int socket_fd_, size_t recv_buff_size_);

        /**
         * @brief Construct a new TCPHandler object using provided address and port of the server.
         * 
         * @param address Address of the server.
         * @param port Port of the server.
         * @param recv_buff_size_ Size of the receive buffer.
         */
        TCPHandler(std::string& address, types::port_t port, size_t recv_buff_size_);

        ~TCPHandler();

        /**
         * @brief Read subsequent n bytes from the TCP stream, convert endianness if needed and
         * put the bytes into the provided buffer. Because of endianness convertion, the method
         * supports only 1, 2, 4 and 8 as the value of n. Moreover, note that this method 
         * performs a read on the socket only if it has to (lazily).
         * 
         * @param n Number of bytes to be read.
         * @param buff Pointer to buffer of size at least N in which read bytes are returned.
         * @throws TCPError.
         */ 
        void read_n_bytes(size_t n, uint8_t* buff);
        
        /**
         * @brief Convert endianness if needed and send n bytes from the buffer. Because of 
         * endianness convertion, the method supports only 1, 2, 4 and 8 as the value of n.
         * 
         * @param n Number of bytes to send.
         * @param buff Pointer to data to be sent.
         * @throws TCPError.
         */
        void send_n_bytes(size_t n, uint8_t* buff);

        // Delete copy constructor and copy assignment.
        TCPHandler(TCPHandler const&) = delete;
        void operator=(TCPHandler const&) = delete;

    private:
        int socket_fd;
        uint8_t *recv_buff;
        size_t recv_buff_size;
        std::deque<char> recv_deque;

        /**
         * @brief Sets up a TCP connection. Sets TCP_NODELAY option for instant message outbound.
         * 
         * @param address Address of the server.
         * @param port Port of the server.
         * @return int File descriptor of the connected socket.
         */
        int set_up_tcp_connection(std::string& address, types::port_t port);

        /**
         * @brief Allocates space for a buffer.
         * 
         * @param n Size of the buffer.
         * @return char* Pointer to the buffer.
         */
        uint8_t* allocate_buffer_space(size_t n);

        /**
         * @brief Reads from the TCP stream as long as there are not enough bytes in recv_deque.
         * 
         * @param n Minimum number of bytes in recv_deque when returning.
         * @throws TCPError.
         */
        void return_when_n_bytes_in_deque(size_t n);
};

#endif // NETWORK_HANDLER_H