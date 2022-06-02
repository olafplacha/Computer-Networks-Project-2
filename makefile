SOURCE_CLIENT = client.cpp config.h messages.h messages.cpp parser.h parser.cpp network_handler.h network_handler.cpp message_manager.h message_manager.cpp game.h game.cpp lobby.h lobby.cpp
SOURCE_SERVER = server.cpp config.h messages.h messages.cpp parser.h parser.cpp network_handler.h network_handler.cpp message_manager.h message_manager.cpp game.h game.cpp lobby.h lobby.cpp connection_acceptor.h connection_acceptor.cpp
SOURCE_TEST0 = accepted_player_container_test.cpp config.h messages.h messages.cpp parser.h parser.cpp network_handler.h network_handler.cpp message_manager.h message_manager.cpp game.h game.cpp lobby.h lobby.cpp accepted_player_container.h accepted_player_container.cpp

# CFLAGS = -pthread -Wall -Wextra -Wconversion -Werror -g -std=gnu++20
CFLAGS = -pthread -Wall -Wextra -Wconversion -g -std=gnu++20
CC = g++-11

all: client server

client:
	$(CC) $(SOURCE_CLIENT) $(CFLAGS) -o robots-client

server:
	$(CC) $(SOURCE_SERVER) $(CFLAGS) -o robots-server

test0:
	$(CC) $(SOURCE_TEST0) $(CFLAGS) -o test

clean:
	-rm -f *.o robots-client
