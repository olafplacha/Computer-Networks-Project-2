#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include <variant>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include "network_handler.h"

enum class Direction : std::underlying_type_t<std::byte> { Up, Right, Down, Left };

struct Join {
    std::string name;

    Join() = default;
    Join(std::string&);
    void serialize(TCPHandler&);
};

struct PlaceBomb  {};

struct PlaceBlock {};

struct Move {
    Direction direction;

    Move() = default;
    Move(Direction direction_);
    void serialize(TCPHandler&);
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
    void serialize(UDPHandler& handler);
};

struct AcceptedPlayer {
    types::player_id_t id;
    Player player;

    AcceptedPlayer() = default;
    AcceptedPlayer(TCPHandler&);
};

struct GameStarted {
    std::map<types::player_id_t, Player> players;

    GameStarted() = default;
    GameStarted(TCPHandler&);
};

struct Position {
    types::size_xy_t x;
    types::size_xy_t y;

    Position() = default;
    Position(TCPHandler&);

    bool operator==(const Position&) const;

    void serialize(UDPHandler&);

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

struct Bomb {
    Position position;
    types::bomb_timer_t timer;
    /* Not serialized */
    types::bomb_id_t id;

    void serialize(UDPHandler&);
};

struct Lobby {
    public:
        Lobby() = default;
        /* Instantiate Lobby based on the first message from the server. */
        Lobby(Hello&);
        /* Change Lobby's state when a new player is accepted */
        void accept(AcceptedPlayer&);
        void serialize(UDPHandler&);

    private:
        std::string server_name;
        types::players_count_t players_count;
        types::size_xy_t size_x;
        types::size_xy_t size_y;
        types::game_length_t game_length;
        types::explosion_radius_t explosion_radius;
        types::bomb_timer_t bomb_timer;
        std::map<types::player_id_t, Player> players;    
};

class Game {
    public:
        Game() = default;
        /* Instantiate Game based on the messages from the server. */
        Game(Hello&, GameStarted&);
        /* Change Game's state based on the Turn received. */
        void apply_turn(Turn&);
        void serialize(UDPHandler&);

    private:
        void apply_event(BombPlaced&);
        void apply_event(BombExploded&);
        void apply_event(PlayerMoved&);
        void apply_event(BlockPlaced&);

        void decrease_bomb_timers();
        void update_scores();
        void find_explosions(const Bomb&);
        void explode_one_direction(const Position&, types::coord_t, types::coord_t);
        
        /* Serialized */
        std::string server_name;
        types::size_xy_t size_x;
        types::size_xy_t size_y;
        types::game_length_t game_length;
        types::turn_t turn;
        std::map<types::player_id_t, Player> players;
        std::map<types::player_id_t, Position> player_positions;
        std::unordered_set<Position, Position::HashFunction> blocks;
        std::vector<Bomb> bombs;
        std::unordered_set<Position, Position::HashFunction> explosions;
        std::map<types::player_id_t, types::score_t> scores;
        /* Not serialized. */
        types::bomb_timer_t bomber_timer;
        types::explosion_radius_t explosion_radius;
        std::set<types::player_id_t> turn_robots_destroyed;
        std::unordered_set<Position, Position::HashFunction> turn_blocks_destroyed;
        std::set<Position> turn_explosions;
};

namespace serverClientCodes {
    const types::message_id_t join = 0;
    const types::message_id_t placeBomb = 1;
    const types::message_id_t placeBlock = 2;
    const types::message_id_t move = 3;
}

namespace guiClientCodes {
    const types::message_id_t lobby = 0;
    const types::message_id_t game = 1;
}

/* Messages sent from client to server. */
using ClientMessage = std::variant<Join, PlaceBomb, PlaceBlock, Move>;
/* Messages sent from server to client. */
using ServerMessage = std::variant<Hello, AcceptedPlayer, GameStarted, Turn, GameEnded>;
/* Messages sent from client to GUI. */
using DrawMessage = std::variant<Lobby, Game>;
/* Messages sent from GUI to client. */
using InputMessage = std::variant<PlaceBomb, PlaceBlock, Move, InvalidMessage>;

class ClientMessageManager
{
    public:
        ClientMessageManager(TCPHandler&, UDPHandler&);

        /**
         * @brief Reads another message from the server.
         * 
         * @return ServerMessage Message from the server.
         */
        ServerMessage read_server_message();
        InputMessage read_gui_message();

        void send_server_message(Join&);
        void send_server_message(PlaceBomb&);
        void send_server_message(PlaceBlock&);
        void send_server_message(Move&);
        void send_server_message(InvalidMessage&);

        void send_gui_message(Lobby&);
        void send_gui_message(Game&);

        /* Delete copy constructor and copy assignment. */
        ClientMessageManager(ClientMessageManager const&) = delete;
        void operator=(ClientMessageManager const&) = delete;

    private:
        TCPHandler& tcp_handler;
        UDPHandler& udp_handler;
};

#endif // MESSAGES_H