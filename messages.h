#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <variant>
#include <map>
#include <vector>
#include "config.h"

enum Direction { Up, Right, Left, Down };

struct Join {
    std::string name;
};

struct PlaceBomb {};

struct PlaceBlock {};

struct Move {
    Direction direction;
};

struct Hello {
    std::string server_name;
    types::players_count_t players_count;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::explosion_radius_t explosion_radius;
    types::bomb_timer_t bomber_timer;
};

struct Player {
    std::string name;
    std::string address;
};

struct AcceptedPlayer {
    types::player_id_t id;
    Player player;
};

struct GameStarted {
    std::map<types::player_id_t, Player> players;
};

struct Position {
    types::size_xy_t x;
    types::size_xy_t y;
};

struct BombPlaced {
    types::bomb_id_t id;
    Position position;
};

struct BombExploded {
    types::bomb_id_t id;
    std::vector<types::player_id_t> robots_destroyed;
    std::vector<Position> blocks_destroyed;
};

struct PlayerMoved {
    types::player_id_t id;
    Position position;
};

struct BlockPlaced {
    Position position;
};

using Event = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

struct Turn {
    types::turn_t turn;
    std::vector<Event> events;
};

struct GameEnded {
    std::map<types::player_id_t, types::score_t> scores;
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

// Types of messages sent over the network.
using ClientMessage = std::variant<Join, PlaceBomb, PlaceBlock, Move>;
using ServerMessage = std::variant<Hello, AcceptedPlayer, GameStarted, Turn, GameEnded>;
using DrawMessage = std::variant<Lobby, Game>;
using InputMessage = std::variant<PlaceBomb, PlaceBlock, Move>;

#endif // MESSAGES_H