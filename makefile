SOURCE_CLIENT = client.cpp config.h messages.h messages.cpp parser.h parser.cpp network_handler.h network_handler.cpp message_manager.h message_manager.cpp
SOURCE_SERVER = server.cpp config.h parser.h parser.cpp

# CFLAGS = -pthread -Wall -Wextra -Wconversion -Werror -g -std=gnu++20
CFLAGS = -pthread -Wall -Wextra -Wconversion -g -std=gnu++20
CC = g++-11

all: client server

client:
	$(CC) $(SOURCE_CLIENT) $(CFLAGS) -o robots-client

server:
	$(CC) $(SOURCE_SERVER) $(CFLAGS) -o robots-server

clean:
	-rm -f *.o robots-client
