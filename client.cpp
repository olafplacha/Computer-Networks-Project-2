#include <iostream>
// #include "parser.h"
#include <limits>
#include <map>
#include "config.h"


int main(int argc, char* argv[]) 
{
    const std::map<std::string, char> REQUIRED_OPTIONS = {
        {"GUI_ADDRESS", 'd'},
        {"PLAYER_NAME", 'n'},
        {"PORT", 'p'},
        {"SERVER_NAME", 's'},
    };

    

    std::cout << std::numeric_limits<types::bomb_timer_t>::max() << '\n';
    return 0;
}