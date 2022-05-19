#include <iostream>
#include "parser.h"
#include <limits>
#include <map>
#include "config.h"
#include <string>

int main(int argc, char* argv[]) 
{

    options_client op = parse_client(argc, argv);
    std::cout << op.gui_address << '\n';
    std::cout << op.gui_port << '\n';
    std::cout << op.player_name << '\n';
    std::cout << op.port << '\n';
    std::cout << op.server_address << '\n';
    std::cout << op.server_port << '\n';

    return 0;
}