#ifndef NETWORK_HANDLER_H
#define NETWORK_HANDLER_H

#include <deque>

/**
 * @brief Class wrapping reading and writing on a TCP socket.
 * 
 */
class TCPHandler {
    public:
        /**
         * @brief Construct a new TCPHandler object.
         * 
         * @param socket_ Connected TCP socket file descriptor.
         * @param recv_buff_size_ Size of the receive buffer.
         */
        TCPHandler(int socket_, int recv_buff_size_);

        ~TCPHandler();

        /**
         * @brief Read subsequent N bytes from the TCP stream. Note that
         * this method performs a read on the socket only if it has to.
         * 
         * @param n Number of bytes to be read.
         * @param buff Pointer to buffer of size at least N.
         * @return False iff the peer was disconnected.
         */ 
        bool read_n_bytes(int n, char* buff);

        // Delete copy constructor and copy assignment.
        TCPHandler(TCPHandler const&) = delete;
        void operator=(TCPHandler const&) = delete;

    private:
        int socket;
        char *recv_buff;
        int recv_buff_size;
        std::deque<char> recv_deque;
};

#endif // NETWORK_HANDLER_H