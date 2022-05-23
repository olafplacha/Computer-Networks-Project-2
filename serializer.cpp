#include <iostream>
#include <functional>
#include <inttypes.h>
#include <arpa/inet.h>
#include "serializer.h"
#include "config.h"

template<typename T>
static T read_element(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];

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
static std::vector<T> read_vector(TCPHandler& handler, std::function<T,(TCPHandler&)> f)
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

static Position read_position(TCPHandler& handler)
{
    Position message;

    message.x = read_element<types::size_xy_t>(handler);
    message.y = read_element<types::size_xy_t>(handler);

    return message;
}

static BombPlaced read_bomb_placed(TCPHandler& handler)
{
    BombPlaced message;

    message.id = read_element<types::bomb_id_t>(handler);
    message.position = read_position(handler);

    return message;
}

static BombExploded read_bomb_exploded(TCPHandler& handler)
{
    BombExploded message;

    message.id = read_element< // TODO: think about placing object creation closer to structs.
}

static Event read_event(TCPHandler& handler)
{
    types::message_id_t message_id = read_element<types::message_id_t>(handler);

    switch (message_id)
    {
    case 0:
        return read_bomb_placed(handler);
    case 1:
        return read_bomb_exploded(handler);
    case 2:
        return read_player_moved(handler);
    case 3:
        return read_block_placed(handler);
    default:
        throw std::runtime_error("Unknown message received from the server!");
    }
}

static Hello read_hello_message(TCPHandler& handler)
{
    Hello message;

    message.server_name = read_string(handler);
    message.players_count = read_element<types::players_count_t>(handler);
    message.size_x = read_element<types::size_xy_t>(handler);
    message.size_y = read_element<types::size_xy_t>(handler);
    message.game_length = read_element<types::game_length_t>(handler);
    message.explosion_radius = read_element<types::explosion_radius_t>(handler);
    message.bomber_timer = read_element<types::bomb_timer_t>(handler);

    return message;
}

static Player read_player(TCPHandler& handler)
{
    Player p;

    p.name = read_string(handler);
    p.address = read_string(handler);

    return p;
}

static AcceptedPlayer read_accepted_player_message(TCPHandler& handler)
{
    AcceptedPlayer message;

    message.id = read_element<types::player_id_t>(handler);
    message.player = read_player(handler);

    return message;
}

static GameStarted read_game_started_message(TCPHandler& handler)
{
    GameStarted message;

    message.players = read_map<types::player_id_t, Player>(handler, 
        &read_element<types::player_id_t>, &read_player);

    return message;
}

static Turn read_turn_message(TCPHandler& handler)
{
    Turn message;

    message.turn = read_element<types::turn_t>(handler);
    message.events = read_vector

    return message;
}

static GameEnded read_game_ended_message(TCPHandler& handler)
{
    GameEnded message;

    // TODO.
    return message;
}

ServerMessage read_client_server_message(TCPHandler& handler)
{
    types::message_id_t message_id = read_element<types::message_id_t>(handler);

    switch (message_id)
    {
    case 0:
        return read_hello_message(handler);
    case 1:
        return read_accepted_player_message(handler);
    case 2:
        return read_game_started_message(handler);
    case 3:
        return read_turn_message(handler);
    case 4:
        return read_game_ended_message(handler);
    default:
        throw std::runtime_error("Unknown message received from the server!");
    }
}
