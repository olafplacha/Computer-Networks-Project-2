#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <variant>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include "network_handler.h"
#include "parser.h"

enum class Direction : std::underlying_type_t<std::byte> { Up, Right, Down, Left };

struct Join {
    std::string name;

    Join() = default;
    Join(std::string&);
    Join(TCPHandler&);
    void serialize(TCPHandler&) const;
};

struct PlaceBomb  {};

struct PlaceBlock {};

struct Move {
    Direction direction;

    Move() = default;
    Move(Direction direction_);
    Move(TCPHandler&);
    void serialize(TCPHandler&) const;
};

struct InvalidMessage {};

struct Hello {
    std::string server_name;
    types::players_count_t players_count;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::explosion_radius_t explosion_radius;
    types::bomb_timer_t bomb_timer;

    Hello() = default;
    Hello(const options_server&);
    Hello(TCPHandler& handler);
    void serialize(TCPHandler&) const;
};

struct Player {
    std::string name;
    std::string address;

    Player() = default;
    Player(TCPHandler&);
    void serialize(UDPHandler&) const;
    void serialize(TCPHandler&) const;
};

struct AcceptedPlayer {
    types::player_id_t id;
    Player player;

    AcceptedPlayer() = default;
    AcceptedPlayer(TCPHandler&);
    void serialize(TCPHandler&) const;
};

struct GameStarted {
    std::map<types::player_id_t, Player> players;

    GameStarted() = default;
    GameStarted(TCPHandler&);
    void serialize(TCPHandler&) const;
};

struct Position {
    types::size_xy_t x;
    types::size_xy_t y;

    Position() = default;
    Position(TCPHandler&);

    bool operator==(const Position&) const;

    void serialize(UDPHandler&) const;
    void serialize(TCPHandler&) const;

    struct HashFunction
    {
        size_t operator()(const Position& p) const
        {
            size_t xHash = std::hash<int>()(p.x);
            size_t yHash = std::hash<int>()(p.y) << 1;
            return xHash ^ yHash;
        }
    };
};

struct BombPlaced {
    types::bomb_id_t id;
    Position position;

    BombPlaced() = default;
    BombPlaced(TCPHandler& handler);
    void serialize(TCPHandler&) const;
};

struct BombExploded {
    types::bomb_id_t id;
    std::vector<types::player_id_t> robots_destroyed;
    std::vector<Position> blocks_destroyed;

    BombExploded() = default;
    BombExploded(TCPHandler& handler);
    void serialize(TCPHandler&) const;
};

struct PlayerMoved {
    types::player_id_t id;
    Position position;

    PlayerMoved() = default;
    PlayerMoved(TCPHandler& handler);
    void serialize(TCPHandler&) const;
};

struct BlockPlaced {
    Position position;

    BlockPlaced() = default;
    BlockPlaced(TCPHandler& handler);
    void serialize(TCPHandler&) const;
};

using Event = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

struct Turn {
    types::turn_t turn;
    std::vector<Event> events;

    Turn() = default;
    Turn(TCPHandler& handler);
    void serialize(TCPHandler&) const;
};

struct GameEnded {
    std::map<types::player_id_t, types::score_t> scores;

    GameEnded() = default;
    GameEnded(TCPHandler& handler);
    void serialize(TCPHandler&) const;
};

struct Bomb {
    Position position;
    types::bomb_timer_t timer;

    void serialize(UDPHandler&) const;
};

struct LobbyMessage {
    std::string server_name;
    types::players_count_t players_count;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::explosion_radius_t explosion_radius;
    types::bomb_timer_t bomb_timer;
    std::map<types::player_id_t, Player> players;

    void serialize(UDPHandler&) const;
};

struct GameMessage {
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

    void serialize(UDPHandler&) const;
};

/* Codes of messages sent from client to server. */
namespace serverClientCodes {
    const types::message_id_t join = 0;
    const types::message_id_t placeBomb = 1;
    const types::message_id_t placeBlock = 2;
    const types::message_id_t move = 3;
}

/* Codes of messages sent from client to gui. */
namespace guiClientCodes {
    const types::message_id_t lobby = 0;
    const types::message_id_t game = 1;
}

/* Codes of messages sent from gui to client. */
namespace clientGuiCodes {
    const types::message_id_t placeBomb = 0;
    const types::message_id_t placeBlock = 1;
    const types::message_id_t move = 2;
}

/* Codes of messages sent from server to client. */
namespace clientServerCodes {
    const types::message_id_t hello = 0;
    const types::message_id_t acceptedPlayer = 1;
    const types::message_id_t gameStarted = 2;
    const types::message_id_t turn = 3;
    const types::message_id_t gameEnded = 4;
}

/* Codes of specific events. */
namespace eventCodes {
    const types::message_id_t bombPlaced = 0;
    const types::message_id_t bombExploded = 1;
    const types::message_id_t playerMoved = 2;
    const types::message_id_t blockPlaced = 3;
}

/* Messages sent from client to server. */
using ClientMessage = std::variant<Join, PlaceBomb, PlaceBlock, Move>;
/* Messages sent from server to client. */
using ServerMessage = std::variant<Hello, AcceptedPlayer, GameStarted, Turn, GameEnded>;
/* Messages sent from client to GUI. */
using DrawMessage = std::variant<LobbyMessage, GameMessage>;
/* Messages sent from GUI to client. */
using InputMessage = std::variant<PlaceBomb, PlaceBlock, Move, InvalidMessage>;

#endif // MESSAGES_H