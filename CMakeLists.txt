cmake_minimum_required(VERSION 3.16)
project(Computer_Networks_Project_2)

set(CMAKE_CXX_COMPILER "gcc")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-pthread -Wall -Wextra -Wconversion -Werror -O2")

add_executable(robots-client src/client.cpp src/config/parser.cpp src/config/parser.h src/config/config.h src/game_logic/game.cpp src/game_logic/game.h src/game_logic/lobby.cpp src/game_logic/lobby.h src/network/message_manager.cpp src/network/message_manager.h src/network/messages.cpp src/network/messages.h src/network/network_handler.cpp src/network/network_handler.h src/concurrency/move_container.cpp src/concurrency/move_container.h)
add_executable(robots-server src/server.cpp src/config/parser.cpp src/config/parser.h src/concurrency/accepted_player_container.cpp src/concurrency/accepted_player_container.h src/config/config.h src/network/connection_acceptor.cpp src/network/connection_acceptor.h src/game_logic/game.cpp src/game_logic/game.h src/network/message_manager.cpp src/network/message_manager.h src/network/messages.cpp src/network/messages.h src/concurrency/move_container.cpp src/concurrency/move_container.h src/network/network_handler.cpp src/network/network_handler.h src/config/parser.cpp src/config/parser.h src/concurrency/turn_container.cpp src/concurrency/turn_container.h)