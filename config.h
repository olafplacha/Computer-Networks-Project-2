#ifndef COMMON_H
#define COMMON_H

#include <inttypes.h>
#include <map>
#include <string>

namespace types
{
    // Common.
    using port_t = uint16_t;

    // Server-specific.
    using bomb_timer_t = uint16_t;
    using players_count_t = uint8_t;
    using turn_duration_t = uint64_t;
    using explosion_radius_t = uint16_t;
    using initial_blocks_t = uint16_t;
    using game_length_t = uint16_t;
    using seed_t = uint32_t;
    using size_xy_t = uint16_t;
}

namespace usage
{
    const std::string CLIENT_USAGE = "-d gui-address -n player-name -p port -s server-address\n";
    const std::string CLIENT_HELP = "This is a placeholder for help info...\n";
}

namespace options
{
    // Common.
    const char HELP = 'h';

    // Client-specific.
    const char CLIENT_OPTSTRING[] = "d:hn:p:s:";
    const char GUI_ADDRESS = 'd';
    const char PLAYER_NAME = 'n';
    const char PORT = 'p';
    const char SERVER_NAME = 's';
    const int CLIENT_REQUIRED = 4;
    const char ADDRESS_DELIMITER = ':';
}

#endif