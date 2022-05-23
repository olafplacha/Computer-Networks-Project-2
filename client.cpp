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
#include "serializer.h"

int main(int argc, char* argv[]) 
{

    options_client op = parse_client(argc, argv);
    // std::cout << op.gui_address << '\n';
    // std::cout << op.gui_port << '\n';
    // std::cout << op.player_name << '\n';
    // std::cout << op.port << '\n';
    // std::cout << op.server_address << '\n';
    // std::cout << op.server_port << '\n';

    uint8_t buff[100] = {0, 49, 240, 159, 145, 169, 240, 159, 143, 188, 226, 128, 141, 240, 159, 145, 169, 240, 159, 143, 188, 226, 128, 141, 240, 159, 145, 167, 240, 159, 143, 188, 226, 128, 141, 240, 159, 145, 166, 240, 159, 143, 188, 240, 159, 135, 181, 240, 159, 135, 177, 99, 70, 10, 0, 10, 0, 69, 0, 1, 0, 2};
    TCPHandler tcp_handler(op.server_address, op.server_port, 1000);
    for (size_t i = 0; i < 64; i++)
    {
        // std::cout << (unsigned) buff[i] << '\n';
        tcp_handler.send_n_bytes(1, buff+i);
    }
    
    ServerMessage mess = read_client_server_message(tcp_handler);
    std::cout << "Read first mess\n";
    std::cout << "-------------------\n\n";
    Hello h = std::get<Hello>(mess);
    std::cout << h.server_name << '\n';
    std::cout << "-------------------\n\n";
    std::cout << (unsigned) h.players_count << '\n';
    std::cout << "-------------------\n\n";
    std::cout << (unsigned) h.size_x << '\n';
    std::cout << "-------------------\n\n";
    std::cout << (unsigned) h.size_y << '\n';
    std::cout << "-------------------\n\n";
    std::cout << (unsigned) h.game_length << '\n';
    std::cout << "-------------------\n\n";
    std::cout << (unsigned) h.explosion_radius << '\n';
    std::cout << "-------------------\n\n";
    std::cout << (unsigned) h.bomber_timer << '\n';

    // uint8_t* buff = (uint8_t *) malloc(100);
    // buff[0] = 1;
    // buff[1] = 2;
    // buff[2] = 3;
    // std::cout << "----------------\n";
    // for (size_t i = 0; i < 5; i++)
    // {
    //     std::cout << i << ": " << (unsigned) buff[i] << '\n';
    // }
    
    // convert_network_to_host_byte_order(buff, 8);
    // std::cout << "----------------\n";
    // for (size_t i = 0; i < 10; i++)
    // {
    //     std::cout << i << ": " << (unsigned) buff[i] << '\n';
    // }

    // char buff[1000];
    // memcpy(buff, op.player_name.c_str(), op.player_name.size());
    // TCPHandler tcp_handler(op.server_address, op.server_port, 1000);
    // tcp_handler.send_n_bytes(op.player_name.size(), buff);
    // std::cout << "size: " << op.player_name.size() << '\n';
    // std::cout << "sent my name!\n";

    // size_t N = 4*op.player_name.size();
    // tcp_handler.read_n_bytes(N, buff);
    // buff[N] = '\0';
    // std::cout << "read my name!\n";
    // std::cout << "buff: " << buff << '\n';

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