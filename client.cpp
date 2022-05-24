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

    // std::cout << sizeof(Move) << '\n';
    // std::cout << (unsigned) Direction::Right << '\n';

    options_client op = parse_client(argc, argv);
    // std::cout << op.gui_address << '\n';
    // std::cout << op.gui_port << '\n';
    // std::cout << op.player_name << '\n';
    // std::cout << op.port << '\n';
    // std::cout << op.server_address << '\n';
    // std::cout << op.server_port << '\n';

    TCPHandler tcp_handler(op.server_address, op.server_port, TCP_BUFF_SIZE);
    UDPHandler udp_handler(op.port, op.gui_address, op.gui_port, UDP_BUFF_SIZE);
    ClientMessager messager(tcp_handler, udp_handler);

    // while (true)
    // {
    //     InputMessage m = messager.read_gui_message();
    //     if (std::holds_alternative<PlaceBomb>(m)) {
    //         std::cout << "PlaceBomb\n";
    //     }
    //     else if (std::holds_alternative<PlaceBlock>(m)) {
    //         std::cout << "PlaceBlock\n";
    //     }
    //     else if (std::holds_alternative<Move>(m)) {
    //         std::cout << "Move " << static_cast<int>(std::get<Move>(m).direction) << '\n';
    //     }
    //     else if (std::holds_alternative<InvalidMessage>(m)) {
    //         std::cout << "InvalidMessage\n";
    //     }
    //     else {
    //         exit(EXIT_FAILURE);
    //     }
    // }

    Join message(op.player_name);
    messager.send_server_message(message);

    // while (true) {
    //     size_t n = udp.read_incoming_packet();
    //     for (size_t i = 0; i < n; i++)
    //     {
    //         char c = udp.read_next_packet_element<char>();
    //         udp.append_to_outcoming_packet<char>(c);
    //     }
    //     std::cout << "Flushing...\n";
    //     udp.flush_outcoming_packet();
    // }

    // uint8_t buff[100] = {3, 0, 44, 0, 0, 0, 3, 2, 3, 0, 2, 0, 4, 2, 4, 0, 3, 0, 5, 0, 0, 0, 0, 5, 0, 5, 0, 7};

    // TCPHandler tcp_handler(op.server_address, op.server_port, TCP_BUFF_SIZE);
    // for (size_t i = 0; i < 64; i++)
    // {
    //     tcp_handler.send_n_bytes(1, buff+i);
    // }
    
    // ClientMessager client_messager(tcp_handler);
    // ServerMessage mess = client_messager.read_server_message();

    // if (std::holds_alternative<Turn>(mess)) {
    //     std::cout << "Type ok!\n";
    // }

    // std::cout << "Read first mess\n";
    // std::cout << "-------------------\n\n";
    // Hello h = std::get<Hello>(mess);
    // std::cout << h.server_name << '\n';
    // std::cout << "-------------------\n\n";
    // std::cout << (unsigned) h.players_count << '\n';
    // std::cout << "-------------------\n\n";
    // std::cout << (unsigned) h.size_x << '\n';
    // std::cout << "-------------------\n\n";
    // std::cout << (unsigned) h.size_y << '\n';
    // std::cout << "-------------------\n\n";
    // std::cout << (unsigned) h.game_length << '\n';
    // std::cout << "-------------------\n\n";
    // std::cout << (unsigned) h.explosion_radius << '\n';
    // std::cout << "-------------------\n\n";
    // std::cout << (unsigned) h.bomber_timer << '\n';

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