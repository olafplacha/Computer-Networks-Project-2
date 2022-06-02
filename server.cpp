#include <iostream>
#include "connection_acceptor.h"
#include "network_handler.h"
#include "config.h"
#include "parser.h"



int main(int argc, char* argv[])
{
    options_server op = parse_server(argc, argv);

    ConnectionAcceptor acceptor(op.port, TCP_BACKLOG_SIZE);
    while (true) {
        int new_fd = acceptor.accept_another_connection();
        std::cout << "Accepted another connectio!\n";
        TCPHandler handler(new_fd, TCP_BUFF_SIZE);
        int i = 1000;
        handler.send_element<int>(i);
    }

    return 0;
}