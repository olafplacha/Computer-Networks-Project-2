#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <variant>
#include <vector>
#include <map>
#include "network_handler.h"

enum class Direction : std::underlying_type_t<std::byte> { Up, Right, Left, Down };

struct Join {
    std::string name;

    void serialize(TCPHandler&);
};

struct PlaceBomb {};

struct PlaceBlock {};

struct Move {
    Direction direction;

    Move() = default;
    Move(Direction& direction_);
};

struct InvalidMessage {};

struct Hello {
    std::string server_name;
    types::players_count_t players_count;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::explosion_radius_t explosion_radius;
    types::bomb_timer_t bomber_timer;

    Hello() = default;
    Hello(TCPHandler& handler);
};

struct Player {
    std::string name;
    std::string address;

    Player() = default;
    Player(TCPHandler& handler);
};

struct AcceptedPlayer {
    types::player_id_t id;
    Player player;

    AcceptedPlayer() = default;
    AcceptedPlayer(TCPHandler& handler);
};

struct GameStarted {
    std::map<types::player_id_t, Player> players;

    GameStarted() = default;
    GameStarted(TCPHandler& handler);
};

struct Position {
    types::size_xy_t x;
    types::size_xy_t y;

    Position() = default;
    Position(TCPHandler& handler);
};

struct BombPlaced {
    types::bomb_id_t id;
    Position position;

    BombPlaced() = default;
    BombPlaced(TCPHandler& handler);
};

struct BombExploded {
    types::bomb_id_t id;
    std::vector<types::player_id_t> robots_destroyed;
    std::vector<Position> blocks_destroyed;

    BombExploded() = default;
    BombExploded(TCPHandler& handler);
};

struct PlayerMoved {
    types::player_id_t id;
    Position position;

    PlayerMoved() = default;
    PlayerMoved(TCPHandler& handler);
};

struct BlockPlaced {
    Position position;

    BlockPlaced() = default;
    BlockPlaced(TCPHandler& handler);
};

using Event = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

struct Turn {
    types::turn_t turn;
    std::vector<Event> events;

    Turn() = default;
    Turn(TCPHandler& handler);
};

struct GameEnded {
    std::map<types::player_id_t, types::score_t> scores;

    GameEnded() = default;
    GameEnded(TCPHandler& handler);
};

struct Lobby {
    std::string server_name;
    types::players_count_t players_count;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::explosion_radius_t explosion_radius;
    types::bomb_timer_t bomb_timer;
    std::map<types::player_id_t, Player> players;
};

struct Bomb {
    Position position;
    types::bomb_timer_t timer;
};

struct Game {
    std::string server_name;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::turn_t turn;
    std::map<types::player_id_t, Player> players;
    std::map<types::player_id_t, Position> player_positions;
    std::vector<Position> blocks;
    std::vector<Bomb> bombs;
    std::vector<Position> explosions;
    std::map<types::player_id_t, types::score_t> scores;
};

/* Messages sent from client to server. */
using ClientMessage = std::variant<Join, PlaceBomb, PlaceBlock, Move>;
/* Messages sent from server to client. */
using ServerMessage = std::variant<Hello, AcceptedPlayer, GameStarted, Turn, GameEnded>;
/* Messages sent from client to GUI. */
using DrawMessage = std::variant<Lobby, Game>;
/* Messages sent from GUI to client. */
using InputMessage = std::variant<PlaceBomb, PlaceBlock, Move, InvalidMessage>;

class ClientMessager
{
    public:
        ClientMessager(TCPHandler&, UDPHandler&);

        /**
         * @brief Reads another message from the server.
         * 
         * @return ServerMessage Message from the server.
         */
        ServerMessage read_server_message();
        InputMessage read_gui_message();

        void send_server_message(ClientMessage&);
        void send_gui_message(DrawMessage&);

        /* Delete copy constructor and copy assignment. */
        ClientMessager(ClientMessager const&) = delete;
        void operator=(ClientMessager const&) = delete;

    private:
        TCPHandler& tcp_handler;
        UDPHandler& udp_handler;
};

#endif // MESSAGES_H