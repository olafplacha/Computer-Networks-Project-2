#include <iostream>
#include <memory>
#include <set>
#include <thread>
#include <csignal>
#include "connection_acceptor.h"
#include "network_handler.h"
#include "config.h"
#include "parser.h"

void handle_tcp_stream_in(TCPHandler::ptr handler) 
{
    
}

void handle_tcp_stream_out(TCPHandler::ptr handler)
{
    try
    {
        std::string s("Siema Tommo, co tam u Ciebie?\n");
        while (true) {
            for (const char& c : s) {
                handler->send_element<char>(c);
            }
            sleep(5);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return;
    }
}

void accept_new_connections(types::port_t port)
{
    std::vector<std::jthread> threads;
    ConnectionAcceptor acceptor(port, TCP_BACKLOG_SIZE);
    
    while (true)
    {
        // Accept another connection.
        try
        {
            std::cout << "Open for new connection\n";
            
            int new_connection_fd = acceptor.accept_another_connection();
            TCPHandler::ptr handler = std::make_shared<TCPHandler>(new_connection_fd, TCP_BUFF_SIZE);

            // Create two threads for data strining in and out of the server.
            std::jthread thread_in{[=]{ handle_tcp_stream_in(handler); }};
            std::jthread thread_out{[=]{ handle_tcp_stream_out(handler); }};
            threads.push_back(std::move(thread_in));
            threads.push_back(std::move(thread_out));

            std::cout << "New connection established\n";
        }
        catch(const TCPAcceptError& e)
        {
            // Continue execution.
            std::cerr << e.what() << '\n';   
        }
    }
}

int main(int argc, char* argv[])
{

    options_server op = parse_server(argc, argv);

    // Create thread for accepting new connection.
    std::thread thread_acceptor{[=]{ accept_new_connections(op.port); }};

    while (true)
    {
        sleep(100);
    }
    thread_acceptor.join();

    return 0;
}