#ifndef MESSAGES_H
#define MESSAGES_H

#include <variant>
#include "network_handler.h"

/* Messages sent from client to server. */
struct Join;
struct PlaceBomb;
struct PlaceBlock;
struct Move;
class ClientMessage = std::variant<Join, PlaceBomb, PlaceBlock, Move>;

/* Messages sent from server to client. */
struct Hello;
struct AcceptedPlayer;
struct GameStarted;
struct Turn;
struct GameEnded;
using ServerMessage = std::variant<Hello, AcceptedPlayer, GameStarted, Turn, GameEnded>;

/* Messages sent from client to GUI. */
struct Lobby;
struct Game;
using DrawMessage = std::variant<Lobby, Game>;

/* Messages sent from GUI to client. */
using InputMessage = std::variant<PlaceBomb, PlaceBlock, Move>;

class ClientMessager
{
    public:
        ClientMessager(TCPHandler&);

        /**
         * @brief Reads another message from the server.
         * 
         * @return ServerMessage Message from the server.
         */
        ServerMessage read_server_message();

        /* Delete copy constructor and copy assignment. */
        ClientMessager(ClientMessager const&) = delete;
        void operator=(ClientMessager const&) = delete;

    private:
        TCPHandler& handler;
};

#endif // MESSAGES_H