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

void serialize_string(std::string& s, std::function<void (uint8_t)> send_int, 
    std::function<void (char)> send_char)
{
    uint8_t n = types::MAX_STR_SIZE;
    if (s.size() < types::MAX_STR_SIZE) {
        n = (uint8_t) s.size();
    }

    // Send data using provided function.
    send_int(n);
    for (size_t i = 0; i < n; i++)
    {
        send_char(s.at(i));
    }
}

Join::Join(std::string& name_)
{
    name = name_;
}

void Join::serialize(TCPHandler& handler)
{
    serialize_string(name, [&](uint8_t t){ return handler.send_element<uint8_t>(t); },
        [&](char t){ handler.send_element<char>(t); });
}

Move::Move(Direction& direction_) : direction(direction_) {}

void Move::serialize(TCPHandler& handler)
{
    handler.send_element<uint8_t>(static_cast<uint8_t>(direction));
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

ClientMessager::ClientMessager(TCPHandler& tcp_handler_, UDPHandler& udp_handler_) : 
    tcp_handler(tcp_handler_), udp_handler(udp_handler_) {}

ServerMessage ClientMessager::read_server_message()
{
    types::message_id_t message_id = read_element<types::message_id_t>(tcp_handler);

    switch (message_id)
    {
    case 0:
        return Hello(tcp_handler);
    case 1:
        return AcceptedPlayer(tcp_handler);
    case 2:
        return GameStarted(tcp_handler);
    case 3:
        return Turn(tcp_handler);
    case 4:
        return GameEnded(tcp_handler);
    default:
        throw std::runtime_error("Unknown message received from the server!");
    }
}

InputMessage ClientMessager::read_gui_message()
{
    // Read another incoming UDP packet.
    size_t packet_size = udp_handler.read_incoming_packet();
    if (packet_size == 0) {
        // Treat empty UDP packet as an invalid message. 
        return InvalidMessage();
    }

    types::message_id_t message_id = udp_handler.read_next_packet_element<types::message_id_t>();

    switch (message_id)
    {
        case 0:
            if (packet_size == 1) return PlaceBomb();
            break;
        case 1:
            if (packet_size == 1) return PlaceBlock();
            break;
        case 2:
            if (packet_size == 1 + sizeof(Move)) {
                uint8_t d_val = udp_handler.read_next_packet_element<u_int8_t>();
                if (d_val < 4) {
                    Direction direction = static_cast<Direction>(d_val);
                    return Move(direction);
                }
            }   
    }
    return InvalidMessage();
}

// Over TCP.
void ClientMessager::send_server_message(Join& message)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::join);
    message.serialize(tcp_handler);
}

void ClientMessager::send_server_message(PlaceBomb& message)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::placeBomb);
    message.
}

void ClientMessager::send_server_message(PlaceBlock& message)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::placeBlock);
}

void ClientMessager::send_server_message(Move& message)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::move);
    message.serialize(tcp_handler);
}



// Over UDP.
void ClientMessager::send_gui_message(DrawMessage& message)
{

    // Flush the packet.
}