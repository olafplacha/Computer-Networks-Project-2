#include <iostream>
#include <set>
#include <string>
#include "parser.h"
#include "config.h"

options_client parse_client(int argc, char* argv[]) 
{
    options_client options;
    std::set<std::string> specified; // zeby upewnic sie ze wystarczajaco duzo specified.

    int opt;
    while ((opt = getopt(argc, argv, options::CLIENT_OPTSTRING)) != -1) 
    {
        switch (opt)
        {
            case options::GUI_ADDRESS:

                break;
            case options::PLAYER_NAME:

                break;
            case options::PORT:

                break;
            case options::SERVER_NAME:

                break;
            case options::HELP:
                std::cout << usage::CLIENT_HELP;
                exit(EXIT_SUCCESS);
            default:
                std::cerr << argv[0] << " " << usage::CLIENT_USAGE;
                exit(EXIT_FAILURE);
        }
    }

}