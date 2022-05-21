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
};

options_client parse_client(int argc, char* argv[]);

#endif // PARSER_H