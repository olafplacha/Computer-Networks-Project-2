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
        TCPHandler(int socket_, size_t recv_buff_size_);

        ~TCPHandler();

        /**
         * @brief Read subsequent N bytes from the TCP stream. Note that this method performs 
         * a read on the socket only if it has to.
         * 
         * @param n Number of bytes to be read.
         * @param buff Pointer to buffer of size at least N in which read bytes are returned.
         * @return true if the read was successful.
         * @return false if the peer disconnected.
         */ 
        bool read_n_bytes(size_t n, char* buff);

        // Delete copy constructor and copy assignment.
        TCPHandler(TCPHandler const&) = delete;
        void operator=(TCPHandler const&) = delete;

    private:
        int socket;
        char *recv_buff;
        size_t recv_buff_size;
        std::deque<char> recv_deque;

        /**
         * @brief Reads from the TCP stream as long as there are not enough bytes in recv_deque.
         * 
         * @param n Minimum number of bytes in recv_deque when returning.
         * @return true if the read was successful.
         * @return false if the peer disconnected.
         */
        bool return_when_n_bytes_in_deque(size_t n);
};

#endif // NETWORK_HANDLER_H