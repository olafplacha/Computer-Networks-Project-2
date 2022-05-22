#include <stdlib.h>
#include <sys/socket.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include "network_handler.h"

TCPHandler::TCPHandler(int socket_, size_t recv_buff_size_) : socket(socket_), 
    recv_buff_size(recv_buff_size_), recv_deque()
{
    recv_buff = (char *) malloc(recv_buff_size_);
    if (recv_buff == NULL) {
        exit(EXIT_FAILURE);
    }
}

TCPHandler::~TCPHandler()
{
    free(recv_buff);
}

bool TCPHandler::return_when_n_bytes_in_deque(size_t n)
{
    // If there are enough bytes in the recv_deque, then do not read on the socket.
    while (recv_deque.size() < n) {
        ssize_t received_bytes = recv(socket, recv_buff, recv_buff_size, 0);
        // Check if read successful.
        if (received_bytes < 0) {
            std::cerr << std::strerror(errno) << '\n';
            exit(EXIT_FAILURE);
        }
        // Check if peer disconnected.
        if (received_bytes == 0) {
            return false;
        }
        // Copy all read bytes to recv_deque.
        for (ssize_t i = 0; i < received_bytes; i++)
        {
            recv_deque.push_back(recv_buff[i]);
        }
    }
    return true;
}

bool TCPHandler::read_n_bytes(size_t n, char* buff)
{
    bool connected = return_when_n_bytes_in_deque(n);
    if (!connected) {
        return false;
    }
    // At this point there are at least n bytes in the deque.
    for (size_t i = 0; i < n; i++)
    {
        buff[i] = recv_deque.front();
        recv_deque.pop_front();
    }
    return true;
}