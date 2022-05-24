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

class UDPError: public std::runtime_error {
    public:
        UDPError(const char* w) : std::runtime_error(w) {}
};

class NetworkHandler {
    public:
        NetworkHandler(size_t recv_buff_size_);

    protected:
        uint8_t* recv_buff;
        size_t recv_buff_size;

        /**
         * @brief Allocates space for a buffer.
         * 
         * @param n Size of the buffer.
         * @return uint8_t* Pointer to the buffer.
         */
        uint8_t* allocate_buffer_space(size_t n);
};

/**
 * @brief Class wrapping reading and writing on a TCP socket. Objects of this class can be
 * instantiated providing previously created socket or by providing name and port of the 
 * server, with which connection will be established during object construction.
 * 
 */
class TCPHandler : public NetworkHandler {
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
         * @param buff Pointer to buffer of size at least n in which read bytes are returned.
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
         * @brief Reads from the TCP stream as long as there are not enough bytes in recv_deque.
         * 
         * @param n Minimum number of bytes in recv_deque when returning.
         * @throws TCPError.
         */
        void return_when_n_bytes_in_deque(size_t n);
};

class UDPHandler : public NetworkHandler {
    public:
        /**
         * @brief Construct a new UDPHandler object.
         * 
         * @param recv_port Port on which the handler listens to incoming UDP packets.
         * @param send_address Address of host to which UDP packets are sent.
         * @param send_port Port of host to which UDP packets are sent.
         * @param recv_buff_size_ Size of the receive buffer.
         */
        UDPHandler(types::port_t recv_port, std::string send_address, types::port_t send_port, 
            size_t recv_buff_size_, size_t send_buff_size_);

        /**
         * @brief Reads incoming UDP packet and puts it into recv_buff.
         * 
         * @return size_t Size of the incoming UDP packet.
         */
        size_t read_incoming_packet();

        /**
         * @brief Reads another element of the UDP packet. Advances pointer to the buffer by
         * the size of the element.
         * 
         * @tparam T Type of the element.
         * @return T Another element of the UDP packet.
         */
        template<typename T>
        T read_next_packet_element();

        template<typename T>
        void append_to_outcoming_packet(T element);

        void flush_outcoming_packet();

        // Delete copy constructor and copy assignment.
        UDPHandler(UDPHandler const&) = delete;
        void operator=(UDPHandler const&) = delete;

    private:
        int recv_socket_fd;
        int send_socket_fd;
        // Size of the last received UDP packet. 
        size_t packet_size;
        // Keeps track of the next element of the packet to be read.
        uint8_t* recv_pointer;
        uint8_t* send_buff;
        // Keeps track of free space in the send buffer.
        uint8_t* send_pointer;
        size_t send_buff_size;

        /**
         * @brief Open a socket for reading UDP packets.
         * 
         * @param port Port on which UDP packets are read.
         * @return int File descriptor of the UDP socket.
         */
        int set_up_udp_listening(types::port_t port);

        /**
         * @brief Open a socket for sending UDP packets and bind it to the provided address.
         * 
         * @param address Address of the host to which UPD packets will be sent.
         * @param port Port of the host to which UDP packets will be sent.
         * @return int File descriptor of the UDP socket.
         */
        int set_up_udp_sending(std::string address, types::port_t port);
};

#endif // NETWORK_HANDLER_H