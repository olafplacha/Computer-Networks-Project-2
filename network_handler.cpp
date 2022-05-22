#include <stdlib.h>
#include "network_handler.h"

TCPHandler::TCPHandler(int socket_, int recv_buff_size_) : socket(socket_), 
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