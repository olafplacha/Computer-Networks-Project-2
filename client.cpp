#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>

#include "config.h"
#include "messages.h"
#include "parser.h"
#include "network_handler.h"

int main(int argc, char* argv[]) 
{

    options_client op = parse_client(argc, argv);
    std::cout << op.gui_address << '\n';
    std::cout << op.gui_port << '\n';
    std::cout << op.player_name << '\n';
    std::cout << op.port << '\n';
    std::cout << op.server_address << '\n';
    std::cout << op.server_port << '\n';

    char buff[1000];
    memcpy(buff, op.player_name.c_str(), op.player_name.size());
    TCPHandler tcp_handler(op.server_address, op.server_port, 1000);
    tcp_handler.send_n_bytes(op.player_name.size(), buff);
    std::cout << "size: " << op.player_name.size() << '\n';
    std::cout << "sent my name!\n";

    size_t N = op.player_name.size()+1;
    tcp_handler.read_n_bytes(N, buff);
    buff[N] = '\0';
    std::cout << "read my name!\n";
    std::cout << "buff: " << buff << '\n';

    // const char* napis = op.player_name.c_str();
    // int i = 0;
    // while (napis[i] != '\0') {
    //     std::cout << (int) (uint8_t) napis[i++] << '\n';
    // }
    
    // struct addrinfo hints;
    // memset(&hints, 0, sizeof hints);
    // hints.ai_family = AF_UNSPEC;
    // hints.ai_socktype = SOCK_DGRAM;
    // hints.ai_protocol = IPPROTO_UDP;

    // struct addrinfo *address_result;
    // if (getaddrinfo(op.gui_address.c_str(), NULL, &hints, &address_result) != 0) {
    //     std::cout << "LIPA\n";
    //     exit(EXIT_FAILURE);
    // }


    // struct addrinfo *p;
    // for (p = address_result; p != NULL; p = p->ai_next) {
    //     void *addr;
    //     if (p->ai_family == AF_INET) {
    //         struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
    //         addr = &(ipv4->sin_addr);
    //     }
    //     else {
    //         struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
    //         addr = &(ipv6->sin6_addr);
    //     }
    //     char ipstr[INET6_ADDRSTRLEN];
    //     inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    //     std::cout << "IP: " << ipstr << "\n";
    // }

    // freeaddrinfo(address_result);

    return 0;
}