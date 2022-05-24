#include <functional>
#include <inttypes.h>
#include <variant>
#include "messages.h"
#include "config.h"

template<typename T>
static T read_element(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_TYPE_SIZE];

    handler.read_n_bytes(sizeof(T), buffer);
    T element = *(T *) buffer;

    return element;
}

template<typename K, typename V>
static std::map<K, V> read_map(TCPHandler& handler, std::function<K(TCPHandler&)> fk,
    std::function<V(TCPHandler&)> fv)
{
    std::map<K, V> m;

    // Insert all keys and values into the map.
    types::map_len_t len = read_element<types::map_len_t>(handler);
    for (size_t i = 0; i < len; i++)
    {
        K key = fk(handler);
        V val = fv(handler);
        m.insert({key, val});
    }
    return m;
}

template<typename T>
static std::vector<T> read_vector(TCPHandler& handler, std::function<T(TCPHandler&)> f)
{
    std::vector<T> v;

    // Insert all elements into the vector.
    types::vec_len_t len = read_element<types::vec_len_t>(handler);
    for (size_t i = 0; i < len; i++)
    {
        T element = f(handler);
        v.push_back(element);
    }
    return v;
}

static std::string read_string(TCPHandler& handler)
{
    std::string s;

    // Read all bytes of the string.
    types::str_len_t len = read_element<types::str_len_t>(handler);
    for (size_t i = 0; i < len; i++)
    {
        char c = read_element<char>(handler);
        s.append(1, c);
    }
    return s;
}

Hello::Hello(TCPHandler& handler)
{
    server_name = read_string(handler);
    players_count = read_element<types::players_count_t>(handler);
    size_x = read_element<types::size_xy_t>(handler);
    size_y = read_element<types::size_xy_t>(handler);
    game_length = read_element<types::game_length_t>(handler);
    explosion_radius = read_element<types::explosion_radius_t>(handler);
    bomber_timer = read_element<types::bomb_timer_t>(handler);
}

Player::Player(TCPHandler& handler)
{
    name = read_string(handler);
    address = read_string(handler);
}

AcceptedPlayer::AcceptedPlayer(TCPHandler& handler)
{
    id = read_element<types::player_id_t>(handler);
    player = Player(handler);
}

GameStarted::GameStarted(TCPHandler& handler)
{
    players = read_map<types::player_id_t, Player>(handler, read_element<types::player_id_t>, 
        [&](TCPHandler& t){ return Player(t); });
}

Position::Position(TCPHandler& handler)
{
    x = read_element<types::size_xy_t>(handler);
    y = read_element<types::size_xy_t>(handler);
}

BombPlaced::BombPlaced(TCPHandler& handler)
{
    id = read_element<types::bomb_id_t>(handler);
    position = Position(handler);
}

BombExploded::BombExploded(TCPHandler& handler)
{
    id = read_element<types::bomb_id_t>(handler);
    robots_destroyed = read_vector<types::player_id_t>(handler, 
        read_element<types::player_id_t>);
    blocks_destroyed = read_vector<Position>(handler, [&](TCPHandler& t){ return Position(t); });
}

PlayerMoved::PlayerMoved(TCPHandler& handler)
{
    id = read_element<types::player_id_t>(handler);
    position = Position(handler);
}

BlockPlaced::BlockPlaced(TCPHandler& handler)
{
    position = Position(handler);
}

using Event = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

static Event read_event(TCPHandler& handler)
{
    types::message_id_t message_id = read_element<types::message_id_t>(handler);

    switch (message_id)
    {
    case 0:
        return BombPlaced(handler);
    case 1:
        return BombExploded(handler);
    case 2:
        return PlayerMoved(handler);
    case 3:
        return BlockPlaced(handler);
    default:
        throw std::runtime_error("Unknown message received from the server!");
    }
} 

Turn::Turn(TCPHandler& handler)
{
    turn = read_element<types::turn_t>(handler);
    events = read_vector<Event>(handler, read_event);
}

GameEnded::GameEnded(TCPHandler& handler)
{
    scores = read_map<types::player_id_t, types::score_t>(handler, 
        read_element<types::player_id_t>, read_element<types::score_t>);
}

ClientMessager::ClientMessager(TCPHandler& handler_) : handler(handler_) {}

ServerMessage ClientMessager::read_server_message()
{
    types::message_id_t message_id = read_element<types::message_id_t>(handler);

    switch (message_id)
    {
    case 0:
        return Hello(handler);
    case 1:
        return AcceptedPlayer(handler);
    case 2:
        return GameStarted(handler);
    case 3:
        return Turn(handler);
    case 4:
        return GameEnded(handler);
    default:
        throw std::runtime_error("Unknown message received from the server!");
    }
}