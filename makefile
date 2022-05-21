SOURCE_CLIENT = client.cpp config.h messages.h parser.h parser.cpp

# CFLAGS = -Wall -Wextra -Wconversion -Werror -g -std=gnu++20
CFLAGS = -Wall -Wextra -Wconversion -g -std=gnu++20
CC = g++-11

all: client

client:
	$(CC) $(SOURCE_CLIENT) $(CFLAGS) -o robots-client

clean:
	-rm -f *.o robots-client
