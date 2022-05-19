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
    // Client-specific.
    const char CLIENT_OPTSTRING[] = "d:hn:p:s";
    const std::map<std::string, char> REQUIRED_OPTIONS = {
        {"GUI_ADDRESS", 'd'},
        {"PLAYER_NAME", 'n'},
        {"PORT", 'p'},
        {"SERVER_NAME", 's'},
    };
    const char HELP = 'h';
    const char ADDRESS_DELIMITER = ':';
}

#endif