#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unistd.h>
#include "config.h"

struct options_client {
    std::string gui_address;
    types::port_t gui_port;
    std::string player_name;
    types::port_t port;
    std::string server_address;
    types::port_t server_port;
};

struct options_server {
    types::bomb_timer_t bomb_timer;
    types::players_count_t players_count;
    types::turn_duration_t turn_duration;
    types::explosion_radius_t explosion_radius;
    types::initial_blocks_t initial_blocks;
    types::game_length_t game_length;
    std::string server_name;
    types::port_t port;
    types::seed_t seed;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
};

options_client parse_client(int argc, char *argv[]);

options_server parse_server(int argc, char *argv[]);

#endif // PARSER_H