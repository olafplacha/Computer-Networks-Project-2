#include "parser.h"

int main(int argc, char* argv[])
{
    options_server op = parse_server(argc, argv);

    return 0;
}