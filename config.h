#ifndef CONFIG_H
#define CONFIG_H

#include <inttypes.h>
#include <string>

const int TCP_BUFF_SIZE = 65536;
const int UDP_BUFF_SIZE = 65536;

namespace types
{
    using port_t = uint16_t;

    using message_id_t = uint8_t;
    using bomb_timer_t = uint16_t;
    using bomb_id_t = uint32_t;
    using players_count_t = uint8_t;
    using player_id_t = uint8_t;
    using turn_duration_t = uint64_t;
    using explosion_radius_t = uint16_t;
    using initial_blocks_t = uint16_t;
    using game_length_t = uint16_t;
    using seed_t = uint32_t;
    using size_xy_t = uint16_t;
    using turn_t = uint16_t;
    using score_t = uint32_t;

    using str_len_t = uint8_t;
    using map_len_t = uint32_t;
    using vec_len_t = uint32_t;

    using coord_t = int64_t;

    const int MAX_TYPE_SIZE = 8;
    const int MAX_STR_SIZE = 255;
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

#endif // CONFIG_H