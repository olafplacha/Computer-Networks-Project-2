/**
 * @author Olaf Placha
 * @brief This module simplifies interaction with Linux Socket API.
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef NETWORK_HANDLER_H
#define NETWORK_HANDLER_H

#include <deque>
#include <string>
#include <cinttypes>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <memory>
#include "../config/config.h"

void convert_network_to_host_byte_order(uint8_t *buffer, size_t n);

void convert_host_to_network_byte_order(uint8_t *buffer, size_t n);

class TCPError : public std::runtime_error {
public:
    explicit TCPError(const char *w) : std::runtime_error(w) {}
};

class UDPError : public std::runtime_error {
public:
    explicit UDPError(const char *w) : std::runtime_error(w) {}
};

class NetworkHandler {
public:
    NetworkHandler(size_t recv_buff_size_, size_t send_buff_size_);

    ~NetworkHandler();

protected:
    uint8_t *recv_buff;
    size_t recv_buff_size;
    uint8_t *send_buff;
    size_t send_buff_size;

    /**
     * @brief Allocates space for a buffer.
     *
     * @param n Size of the buffer.
     * @return uint8_t* Pointer to the buffer.
     */
    static uint8_t *allocate_buffer_space(size_t n);
};

/**
 * @brief Class wrapping reading and writing on a TCP socket. Objects of this class can be
 * instantiated providing previously created socket or by providing name and port of the
 * server, with which connection will be established during object construction.
 *
 */
class TCPHandler : public NetworkHandler {
public:
    using ptr = std::shared_ptr<TCPHandler>;

    /**
     * @brief Construct a new TCPHandler object using previously created socket.
     *
     * @param socket_fd_ Connected TCP socket file descriptor.
     * @param buff_size_ Size of the receive/send buffers.
     */
    TCPHandler(int socket_fd_, size_t buff_size_);

    /**
     * @brief Construct a new TCPHandler object using provided address and port of the server.
     *
     * @param address Address of the server.
     * @param port Port of the server.
     * @param buff_size_ Size of the receive/send buffers.
     */
    TCPHandler(std::string &address, types::port_t port, size_t buff_size_);

    [[nodiscard]] std::string get_peer_name() const;

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
    template<typename T>
    T read_element();

    // Send element over TCP connection.
    template<typename T>
    void send_element(T element);

    // Delete copy constructor and copy assignment.
    TCPHandler(TCPHandler const &) = delete;

    void operator=(TCPHandler const &) = delete;

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
    static int set_up_tcp_connection(std::string &address, types::port_t port);

    /**
     * @brief Reads from the TCP stream as long as there are not enough bytes in recv_deque.
     *
     * @param n Minimum number of bytes in recv_deque when returning.
     * @throws TCPError.
     */
    void return_when_n_bytes_in_deque(size_t n);

    void send_n_bytes(size_t n, uint8_t *buff) const;
};

class UDPHandler : public NetworkHandler {
public:
    /**
     * @brief Construct a new UDPHandler object.
     *
     * @param recv_port Port on which the handler listens to incoming UDP packets.
     * @param send_address Address of host to which UDP packets are sent.
     * @param send_port Port of host to which UDP packets are sent.
     * @param buff_size_ Size of the receive/send buffers.
     */
    UDPHandler(types::port_t recv_port, const std::string &send_address, types::port_t send_port,
               size_t buff_size_);

    ~UDPHandler();

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
    UDPHandler(UDPHandler const &) = delete;

    void operator=(UDPHandler const &) = delete;

private:
    int recv_socket_fd;
    int send_socket_fd;
    // Size of the last received UDP packet.
    size_t packet_size;
    // Keeps track of the next element of the packet to be read.
    uint8_t *recv_pointer;
    // Keeps track of free space in the send buffer.
    uint8_t *send_pointer;

    /**
     * @brief Open a socket for reading UDP packets.
     *
     * @param port Port on which UDP packets are read.
     * @return int File descriptor of the UDP socket.
     */
    static int set_up_udp_listening(types::port_t port);

    /**
     * @brief Open a socket for sending UDP packets and bind it to the provided address.
     *
     * @param address Address of the host to which UPD packets will be sent.
     * @param port Port of the host to which UDP packets will be sent.
     * @return int File descriptor of the UDP socket.
     */
    static int set_up_udp_sending(const std::string &address, types::port_t port);
};

template<typename T>
T TCPHandler::read_element() {
    size_t n = sizeof(T);
    return_when_n_bytes_in_deque(n);
    // At this point there are at least n bytes in the deque.
    uint8_t temp_buff[n];
    for (size_t i = 0; i < n; i++) {
        // Copy n bytes from the deque to the buffer.
        temp_buff[i] = recv_deque.front();
        recv_deque.pop_front();
    }
    // Convert the endianness if needed.
    convert_network_to_host_byte_order(temp_buff, n);
    // Cast the received bytes.
    return *(T *) temp_buff;
}

template<typename T>
void TCPHandler::send_element(T element) {
    // Put the bytes into the send buffer.
    std::memcpy(send_buff, &element, sizeof(T));

    // Convert the endianness if needed.
    convert_host_to_network_byte_order(send_buff, sizeof(T));

    send_n_bytes(sizeof(T), send_buff);
}

template<typename T>
T UDPHandler::read_next_packet_element() {
    // Check if there is enough data left in the buffer.
    if (recv_buff + packet_size < recv_pointer + sizeof(T)) {
        throw UDPError("Attempt to read data out of UDP packet's bound!");
    }

    // Convert the endianness if needed.
    convert_network_to_host_byte_order(recv_pointer, sizeof(T));
    T element = *(T *) recv_pointer;

    // Advance the pointer.
    recv_pointer += sizeof(T);

    return element;
}

template<typename T>
void UDPHandler::append_to_outcoming_packet(T element) {
    // Check if the element will fit into the send buffer.
    if (send_buff + send_buff_size < send_pointer + sizeof(T)) {
        throw UDPError("Data does not fit into the send buffer!");
    }

    std::memcpy(send_pointer, &element, sizeof(T));
    convert_host_to_network_byte_order(send_pointer, sizeof(T));

    // Advance the pointer.
    send_pointer += sizeof(T);
}

#endif // NETWORK_HANDLER_H