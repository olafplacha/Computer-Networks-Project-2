SOURCE_CLIENT = client.cpp config.h messages.h messages.cpp parser.h parser.cpp network_handler.h network_handler.cpp

CFLAGS = -pthread -Wall -Wextra -Wconversion -Werror -g -std=gnu++20
CC = g++-11

all: client

client:
	$(CC) $(SOURCE_CLIENT) $(CFLAGS) -o robots-client

clean:
	-rm -f *.o robots-client
