#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unistd.h>
#include "config.h"

struct options_client
{
    std::string gui_address;
    types::port_t gui_port;
    std::string player_name;
    types::port_t port;
    std::string server_address;
    types::port_t server_port;

    // options_client(std::string& gui_address_, types::port_t gui_port_, std::string& player_name_,
    //     types::port_t port_, std::string& server_address_, types::port_t server_port_) : 
    //     gui_address(std::move(gui_address_)), gui_port(gui_port_), 
    //     player_name(std::move(player_name_)), port(port_), 
    //     server_address(std::move(server_address_)), server_port(server_port_) {};
};

options_client parse_client(int argc, char* argv[]);

#endif