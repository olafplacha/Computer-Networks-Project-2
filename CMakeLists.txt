cmake_minimum_required(VERSION 3.16)
project(Computer_Networks_Project_2)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(robots-client client.cpp parser.cpp parser.h config.h game.cpp game.h lobby.cpp lobby.h message_manager.cpp message_manager.h messages.cpp messages.h network_handler.cpp network_handler.h move_container.cpp move_container.h)
add_executable(robots-server server.cpp parser.cpp parser.h accepted_player_container.cpp accepted_player_container.h config.h connection_acceptor.cpp connection_acceptor.h game.cpp game.h message_manager.cpp message_manager.h messages.cpp messages.h move_container.cpp move_container.h network_handler.cpp network_handler.h parser.cpp parser.h turn_container.cpp turn_container.h)
add_executable(test accepted_player_container_test.cpp parser.h accepted_player_container.cpp accepted_player_container.h config.h messages.cpp messages.h network_handler.cpp network_handler.h)

target_link_libraries(robots-client PRIVATE Threads::Threads)
target_link_libraries(robots-server PRIVATE Threads::Threads)
target_link_libraries(test PRIVATE Threads::Threads)