#include <iostream>
#include <set>
#include <string>
#include <limits>
#include "parser.h"
#include "config.h"

static void exit_wrong_param(const char* program)
{
    std::cerr << "Usage: " << program << " " << usage::CLIENT_USAGE;
    exit(EXIT_FAILURE);
}

static void exit_help(void)
{
    std::cout << usage::CLIENT_HELP;
    exit(EXIT_SUCCESS);
}

static void validate_limit(uint64_t val, uint64_t limit, std::string& message)
{
    if (val > limit) {
        std::cerr << message << " out of range! Should not be greater than " << limit << ".\n";
        exit(EXIT_FAILURE);
    }
}

static uint64_t get_numerical_value(const char* s, std::string& message)
{
    char* tmp;
    uint64_t res = strtoll(s, &tmp, 10);
    if (errno == ERANGE || *tmp != '\0') {
        std::cerr << message << " value not parsable!\n";
        exit(EXIT_FAILURE);
    }
    return res;
}

template<typename T>
static T parse_numerical(const char* s, std::string&& message)
{
    if (s[0] == '\0') {
        std::cerr << message << " cannot be parsed!\n";
        exit(EXIT_FAILURE);
    }
    if (s[0] == '-') {
        std::cerr << message << " cannot be negative!\n";
        exit(EXIT_FAILURE);
    }
    uint64_t t = get_numerical_value(s, message);
    validate_limit(t, std::numeric_limits<T>::max(), message);
    return (T) t;
}

static void parse_address(std::string& addr, types::port_t& port, std::string s, std::string&& message)
{
    auto pos = s.find_last_of(options::ADDRESS_DELIMITER);
    if (pos == std::string::npos) {
        std::cerr << message << " address cannot be parsed!\n";
        exit(EXIT_FAILURE);
    }
    addr = s.substr(0, pos);
    port = parse_numerical<types::port_t>(s.substr(pos + 1).c_str(), message + " port");
}

options_client parse_client(int argc, char* argv[])
{
    options_client options;

    // Validates if all required parameters have been specified.
    std::set<int> specified;

    // Validates if any unknown parameter was specified.
    int counter = 1;

    int opt;
    while ((opt = getopt(argc, argv, options::CLIENT_OPTSTRING)) != -1)
    {
        specified.insert(opt);
        counter += 2;
        switch (opt)
        {
        case options::GUI_ADDRESS:
            parse_address(options.gui_address, options.gui_port, optarg, "GUI");
            break;
        case options::PLAYER_NAME:
            options.player_name = optarg;
            break;
        case options::PORT:
            options.port = parse_numerical<types::port_t>(optarg, "Port");
            break;
        case options::SERVER_NAME:
            parse_address(options.server_address, options.server_port, optarg, "Server");
            break;
        case options::HELP:
            exit_help();
            break;
        default:
            exit_wrong_param(argv[0]);
        }
    }

    // Check if all required parameters have been specified.
    if (argc != counter || specified.size() != options::CLIENT_REQUIRED) {
        exit_wrong_param(argv[0]);
    }

    return options;
}