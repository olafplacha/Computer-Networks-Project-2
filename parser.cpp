#include <iostream>
#include <set>
#include <string>
#include <limits>
#include <chrono>
#include "parser.h"
#include "config.h"

struct required_client
{
    bool gui_address = true;
    bool gui_port = true;
    bool player_name = true;
    bool port = true;
    bool server_address = true;
    bool server_port = true;
};

struct required_server
{
    bool bomb_timer = true;
    bool players_count = true;
    bool turn_duration = true;
    bool explosion_radius = true;
    bool initial_blocks = true;
    bool game_length = true;
    bool server_name = true;
    bool port = true;
    bool seed = false;
    bool size_x = true;
    bool size_y = true;
};

static bool required_specified_client(const required_client &required)
{
    bool result = !required.gui_address &&
                  !required.gui_port &&
                  !required.player_name &&
                  !required.port &&
                  !required.server_address &&
                  !required.server_port;
    return result;
}

static bool required_specified_server(const required_server &required)
{
    bool result = !required.bomb_timer &&
                  !required.players_count &&
                  !required.turn_duration &&
                  !required.explosion_radius &&
                  !required.initial_blocks &&
                  !required.game_length &&
                  !required.server_name &&
                  !required.port &&
                  !required.seed &&
                  !required.size_x &&
                  !required.size_y;

    return result;
}

static void exit_wrong_param(std::string program, const std::string &usage)
{
    std::cerr << "Usage: " << program << " " << usage;
    exit(EXIT_FAILURE);
}

static void exit_help(std::string program, const std::string &help_message)
{
    std::cerr << "Usage: " << program << " " << help_message;
    exit(EXIT_SUCCESS);
}

static void validate_limit(uint64_t val, uint64_t limit, const std::string &message)
{
    if (val > limit)
    {
        std::cerr << message << " out of range! Should not be greater than " << limit << ".\n";
        exit(EXIT_FAILURE);
    }
}

static uint64_t get_numerical_value(const char *s, const std::string &message)
{
    char *tmp;
    uint64_t res = strtoll(s, &tmp, 10);
    if (errno == ERANGE || *tmp != '\0')
    {
        std::cerr << message << " value not parsable!\n";
        exit(EXIT_FAILURE);
    }
    return res;
}

template <typename T>
static T parse_numerical(const char *s, std::string &&message)
{
    if (s[0] == '\0')
    {
        std::cerr << message << " cannot be parsed!\n";
        exit(EXIT_FAILURE);
    }
    if (s[0] == '-')
    {
        std::cerr << message << " cannot be negative!\n";
        exit(EXIT_FAILURE);
    }
    uint64_t t = get_numerical_value(s, message);
    validate_limit(t, std::numeric_limits<T>::max(), message);
    return (T)t;
}

static void parse_address(std::string &addr, types::port_t& port, std::string s, std::string &&message)
{
    auto pos = s.find_last_of(options::ADDRESS_DELIMITER);
    if (pos == std::string::npos)
    {
        std::cerr << message << " address cannot be parsed!\n";
        exit(EXIT_FAILURE);
    }
    addr = s.substr(0, pos);
    port = parse_numerical<types::port_t>(s.substr(pos + 1).c_str(), message + " port");
}

options_client parse_client(int argc, char *argv[])
{
    options_client options;
    required_client required;

    // Validates if any unknown parameter was specified.
    int counter = 1;

    int opt;
    while ((opt = getopt(argc, argv, options::CLIENT_OPTSTRING)) != -1)
    {
        counter += 2;
        switch (opt)
        {
        case options::GUI_ADDRESS:
            parse_address(options.gui_address, options.gui_port, optarg, "GUI");
            required.gui_address = false;
            required.gui_port = false;
            break;
        case options::PLAYER_NAME:
            options.player_name = optarg;
            required.player_name = false;
            break;
        case options::PORT:
            options.port = parse_numerical<types::port_t>(optarg, "Port");
            required.port = false;
            break;
        case options::SERVER_ADDRESS:
            parse_address(options.server_address, options.server_port, optarg, "Server");
            required.server_address = false;
            required.server_port = false;
            break;
        case options::HELP:
            exit_help(argv[0], usage::CLIENT_HELP);
            break;
        default:
            exit_wrong_param(argv[0], usage::CLIENT_USAGE);
        }
    }

    // Check if all required parameters have been specified.
    if (argc != counter || !required_specified_client(required))
    {
        exit_wrong_param(argv[0], usage::CLIENT_USAGE);
    }

    return options;
}

options_server parse_server(int argc, char *argv[])
{
    options_server options;
    required_server required;

    // Get default seed value.
    options.seed = (types::seed_t) std::chrono::system_clock::now().time_since_epoch().count();

    // Validates if any unknown parameter was specified.
    int counter = 1;

    int opt;
    while ((opt = getopt(argc, argv, options::SERVER_OPTSTRING)) != -1)
    {
        counter += 2;
        switch (opt)
        {
        case options::BOMB_TIMER:
            options.bomb_timer = parse_numerical<types::bomb_timer_t>(optarg, "Bomb timer");
            required.bomb_timer = false;
            break;
        case options::PLAYER_COUNT:
            options.players_count = parse_numerical<types::players_count_t>(optarg, "Players count");
            required.players_count = false;
            break;
        case options::TURN_DURATION:
            options.turn_duration = parse_numerical<types::turn_duration_t>(optarg, "Turn duration");
            required.turn_duration = false;
            break;
        case options::EXPLOSION_RADIUS:
            options.explosion_radius = parse_numerical<types::explosion_radius_t>(optarg, "Explosion radius");
            required.explosion_radius = false;
            break;
        case options::INITIAL_BLOCKS:
            options.initial_blocks = parse_numerical<types::initial_blocks_t>(optarg, "Initial blocks");
            required.initial_blocks = false;
            break;
        case options::GAME_LENGTH:
            options.game_length = parse_numerical<types::game_length_t>(optarg, "Game length");
            required.game_length = false;
            break;
        case options::SERVER_NAME:
            options.server_name = optarg;
            required.server_name = false;
            break;
        case options::PORT:
            options.port = parse_numerical<types::port_t>(optarg, "Port");
            required.port = false;
            break;
        case options::SEED:
            options.seed = parse_numerical<types::seed_t>(optarg, "Seed");
            required.seed = false;
            break;
        case options::SIZE_X:
            options.size_x = parse_numerical<types::size_xy_t>(optarg, "Size x");
            required.size_x = false;
            break;
        case options::SIZE_Y:
            options.size_y = parse_numerical<types::size_xy_t>(optarg, "Size y");
            required.size_y = false;
            break;
        case options::HELP:
            exit_help(argv[0], usage::SERVER_HELP);
            break;
        default:
            exit_wrong_param(argv[0], usage::SERVER_USAGE);
        }
    }

    // Check if all required parameters have been specified.
    if (argc != counter || !required_specified_server(required))
    {
        exit_wrong_param(argv[0], usage::SERVER_USAGE);
    }

    return options;
}
