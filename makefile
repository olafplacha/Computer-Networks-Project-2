SOURCE_CLIENT = src/client.cpp src/config/parser.cpp src/config/parser.h src/config/config.h src/game_logic/game.cpp src/game_logic/game.h src/game_logic/lobby.cpp src/game_logic/lobby.h src/network/message_manager.cpp src/network/message_manager.h src/network/messages.cpp src/network/messages.h src/network/network_handler.cpp src/network/network_handler.h src/concurrency/move_container.cpp src/concurrency/move_container.h
SOURCE_SERVER = src/server.cpp src/config/parser.cpp src/config/parser.h src/concurrency/accepted_player_container.cpp src/concurrency/accepted_player_container.h src/config/config.h src/network/connection_acceptor.cpp src/network/connection_acceptor.h src/game_logic/game.cpp src/game_logic/game.h src/network/message_manager.cpp src/network/message_manager.h src/network/messages.cpp src/network/messages.h src/concurrency/move_container.cpp src/concurrency/move_container.h src/network/network_handler.cpp src/network/network_handler.h src/concurrency/turn_container.cpp src/concurrency/turn_container.h

CFLAGS = -pthread -Wall -Wextra -Wconversion -Werror -O2 -std=gnu++20
CC = g++

all: client server

client:
	$(CC) $(SOURCE_CLIENT) $(CFLAGS) -o robots-client

server:
	$(CC) $(SOURCE_SERVER) $(CFLAGS) -o robots-server

clean:
	-rm -f *.o robots-client robots-server