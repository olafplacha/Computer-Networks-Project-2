/**
 * @author Olaf Placha
 * @brief This file provides configuration information about the game logic as well as 
 * about the message protocol.
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <inttypes.h>
#include <string>
#include <vector>

const int TCP_BUFF_SIZE = 65536;
const int UDP_BUFF_SIZE = 65536;
const int TCP_BACKLOG_SIZE = 32;

namespace types {
    const int MAX_TYPE_SIZE = 8;

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
}

namespace usage {
    const std::string CLIENT_USAGE = "-d <GUI_ADDRESS> -n <PLAYER_NAME> -p <PORT> -s <SERVER_ADDRESS>\n";
    const std::string CLIENT_HELP = CLIENT_USAGE + "\nOptions:\n" +
                                    "\t-d\tAddress of GUI: <(host name):(port) or (IPv4):(port) or (IPv6):(port)>.\n" +
                                    "\t-n\tPlayer's name.\n" +
                                    "\t-p\tPort on which client listens for move instruction packets.\n" +
                                    "\t-d\tAddress of server: <(host name):(port) or (IPv4):(port) or (IPv6):(port)>.\n" +
                                    "\t-h\tShows usage information.\n";

    const std::string SERVER_USAGE = std::string("-b <BOMB_TIMER> -c <PLAYERS_COUNT> -d <TURN_DURATION> ") +
                                     "-e <EXPLOSION_RADIUS> -k <INITIAL_BLOCKS> -l <GAME_LENGTH> -n <SERVER_NAME> " +
                                     "-p <PORT> [-s <SEED>] -x <SIZE_X> -y <SIZE_Y>\n";
    const std::string SERVER_HELP = SERVER_USAGE + "\nOptions:\n" +
                                                   "\t-b\tBomb timer.\n" +
                                                   "\t-c\tNumber of players required for the game.\n" +
                                                   "\t-d\tNumber of milliseconds per turn.\n" +
                                                   "\t-e\tExplosion radius.\n" +
                                                   "\t-k\tNumber of initial blocks.\n" +
                                                   "\t-l\tGame length in turns.\n" +
                                                   "\t-n\tServer name.\n" +
                                                   "\t-p\tPort of the server.\n" +
                                                   "\t-s\tRandom seed.\n" +
                                                   "\t-x\tSize x in number of blocks.\n" +
                                                   "\t-y\tSize y in number of blocks.\n";
}

namespace options {
    // Common.
    const char HELP = 'h';
    const char PORT = 'p';
    const char ADDRESS_DELIMITER = ':';

    // Client-specific.
    const char CLIENT_OPTSTRING[] = "d:hn:p:s:";
    const char GUI_ADDRESS = 'd';
    const char PLAYER_NAME = 'n';
    const char SERVER_ADDRESS = 's';

    // Server-specific.
    const char SERVER_OPTSTRING[] = "b:c:d:e:hk:l:n:p:s:x:y:";
    const char BOMB_TIMER = 'b';
    const char PLAYER_COUNT = 'c';
    const char TURN_DURATION = 'd';
    const char EXPLOSION_RADIUS = 'e';
    const char INITIAL_BLOCKS = 'k';
    const char GAME_LENGTH = 'l';
    const char SERVER_NAME = 'n';
    const char SEED = 's';
    const char SIZE_X = 'x';
    const char SIZE_Y = 'y';
}

#endif // CONFIG_H