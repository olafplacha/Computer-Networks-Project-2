#include <functional>
#include <inttypes.h>
#include <variant>
#include <limits>
#include "messages.h"
#include "config.h"

// fk and fv invoked read another element from TCP stream.
template<typename K, typename V>
static std::map<K, V> read_map(TCPHandler& handler, std::function<K()> fk,
                               std::function<V()> fv)
{
    std::map<K, V> m;

    // Insert all keys and values into the map.
    types::map_len_t len = handler.read_element<types::map_len_t>();
    for (size_t i = 0; i < len; i++)
    {
        K key = fk();
        V val = fv();
        m.insert({key, val});
    }
    return m;
}

// f invoked read another element from TCP stream.
template<typename T>
static std::vector<T> read_vector(TCPHandler& handler, std::function<T()> f)
{
    std::vector<T> v;

    // Insert all elements into the vector.
    types::vec_len_t len = handler.read_element<types::vec_len_t>();
    for (size_t i = 0; i < len; i++)
    {
        T element = f();
        v.push_back(element);
    }
    return v;
}

static std::string read_string(TCPHandler& handler)
{
    std::string s;

    // Read all bytes of the string.
    types::str_len_t len = handler.read_element<types::str_len_t>();
    for (size_t i = 0; i < len; i++)
    {
        char c = handler.read_element<char>();
        s.append(1, c);
    }
    return s;
}

static void serialize_string(const std::string& s, std::function<void (types::str_len_t)> send_len,
                      std::function<void (char)> send_char)
{
    // Truncate string if too long.
    types::str_len_t n = std::numeric_limits<types::str_len_t>::max();
    if (s.size() < n) {
        n = (types::str_len_t) s.size();
    }

    // Send data using provided function.
    send_len(n);
    for (size_t i = 0; i < n; i++)
    {
        send_char(s.at(i));
    }
}

template<typename K, typename V>
static void serialize_map(const std::map<K, V>& m, std::function<void (types::map_len_t)> send_len,
                   std::function<void (const K&)> send_key, std::function<void (const V&)> send_val)
{
    // Check if the size of the map is supported.
    size_t size = m.size();
    if (size > std::numeric_limits<types::map_len_t>::max()) {
        throw std::runtime_error("Trying to send a map of unsupported size!");
    }
    types::map_len_t n = (types::map_len_t) size;

    // Send data using provided functions.
    send_len(n);
    for (auto const& [key, value] : m) {
        send_key(key);
        send_val(value);
    }
}

template<typename T>
static void serialize_vector(const std::vector<T>& v, std::function<void (types::vec_len_t)> send_len,
                      std::function<void (const T&)> send_element)
{
    // Check if the size of the vector is supported.
    size_t size = v.size();
    if (size > std::numeric_limits<types::vec_len_t>::max()) {
        throw std::runtime_error("Trying to send a vector of unsupported size!");
    }
    types::vec_len_t n = (types::vec_len_t) size;

    // Send data using provided functions.
    send_len(n);
    for (auto const& element : v) {
        send_element(element);
    }
}

Join::Join(std::string& name_)
{
    name = name_;
}

Join::Join(TCPHandler& handler)
{
    name = read_string(handler);
}

void Join::serialize(TCPHandler& handler) const
{
    serialize_string(name, [&](types::str_len_t t) {
        handler.send_element<types::str_len_t>(t);
    },
    [&](char t) {
        handler.send_element<char>(t);
    });
}

Move::Move(Direction direction_) : direction(direction_) {}

Move::Move(TCPHandler& handler)
{
    uint8_t direction_ = handler.read_element<uint8_t>();
    direction = static_cast<Direction>(direction_);
}

void Move::serialize(TCPHandler& handler) const
{
    handler.send_element<uint8_t>(static_cast<uint8_t>(direction));
}

Hello::Hello(const options_server& op)
{
    server_name = op.server_name;
    players_count = op.players_count;
    size_x = op.size_x;
    size_y = op.size_y;
    game_length = op.game_length;
    explosion_radius = op.explosion_radius;
    bomb_timer = op.bomb_timer;
}

Hello::Hello(TCPHandler& handler)
{
    server_name = read_string(handler);
    players_count = handler.read_element<types::players_count_t>();
    size_x = handler.read_element<types::size_xy_t>();
    size_y = handler.read_element<types::size_xy_t>();
    game_length = handler.read_element<types::game_length_t>();
    explosion_radius = handler.read_element<types::explosion_radius_t>();
    bomb_timer = handler.read_element<types::bomb_timer_t>();
}

void Hello::serialize(TCPHandler& handler) const
{
    serialize_string(server_name, [&](types::str_len_t t) {
        handler.send_element<types::str_len_t>(t);
    },
    [&](char t) {
        handler.send_element<char>(t);
    });

    handler.send_element<types::players_count_t>(players_count);
    handler.send_element<types::size_xy_t>(size_x);
    handler.send_element<types::size_xy_t>(size_y);
    handler.send_element<types::game_length_t>(game_length);
    handler.send_element<types::explosion_radius_t>(explosion_radius);
    handler.send_element<types::bomb_timer_t>(bomb_timer);
}

Player::Player(TCPHandler& handler)
{
    name = read_string(handler);
    address = read_string(handler);
}

void Player::serialize(UDPHandler& handler) const
{
    auto send_len = [&](types::str_len_t t) {
        handler.append_to_outcoming_packet<types::str_len_t>(t);
    };
    auto send_char = [&](char t) {
        handler.append_to_outcoming_packet<char>(t);
    };
    serialize_string(name, send_len, send_char);
    serialize_string(address, send_len, send_char);
}

void Player::serialize(TCPHandler& handler) const
{
    auto send_len = [&](types::str_len_t t) {
        handler.send_element<types::str_len_t>(t);
    };
    auto send_char = [&](char t) {
        handler.send_element<char>(t);
    };
    serialize_string(name, send_len, send_char);
    serialize_string(address, send_len, send_char);
}

AcceptedPlayer::AcceptedPlayer(TCPHandler& handler)
{
    id = handler.read_element<types::player_id_t>();
    player = Player(handler);
}

void AcceptedPlayer::serialize(TCPHandler& handler) const
{
    handler.send_element<types::player_id_t>(id);
    player.serialize(handler);
}

GameStarted::GameStarted(TCPHandler& handler)
{
    players = read_map<types::player_id_t, Player>(handler, [&]() { return handler.read_element<types::player_id_t>(); },
    [&]() {
        return Player(handler);
    });
}

void GameStarted::serialize(TCPHandler& handler) const
{
    auto send_len = [&](types::map_len_t t) {
        handler.send_element<types::map_len_t>(t);
    };
    auto send_key = [&](const types::player_id_t& t) {
        handler.send_element<types::player_id_t>(t);
    };
    auto send_val = [&](const Player& t) {
        t.serialize(handler);
    };
    serialize_map<types::player_id_t, Player>(players, send_len, send_key, send_val);
}

Position::Position(TCPHandler& handler)
{
    x = handler.read_element<types::size_xy_t>();
    y = handler.read_element<types::size_xy_t>();
}

bool Position::operator==(const Position& rhs) const
{
    return x == rhs.x && y == rhs.y;
}

void Position::serialize(UDPHandler& handler) const
{
    handler.append_to_outcoming_packet<types::size_xy_t>(x);
    handler.append_to_outcoming_packet<types::size_xy_t>(y);
}

void Position::serialize(TCPHandler& handler) const
{
    handler.send_element<types::size_xy_t>(x);
    handler.send_element<types::size_xy_t>(y);
}

BombPlaced::BombPlaced(TCPHandler& handler)
{
    id = handler.read_element<types::bomb_id_t>();
    position = Position(handler);
}

void BombPlaced::serialize(TCPHandler& handler) const
{
    handler.send_element<types::message_id_t>(eventCodes::bombPlaced);
    handler.send_element<types::bomb_id_t>(id);
    position.serialize(handler);
}

BombExploded::BombExploded(TCPHandler& handler)
{
    id = handler.read_element<types::bomb_id_t>();
    robots_destroyed = read_vector<types::player_id_t>(handler,
                       [&](){ return handler.read_element<types::player_id_t>(); });
    blocks_destroyed = read_vector<Position>(handler, [&]() {
        return Position(handler);
    });
}

void BombExploded::serialize(TCPHandler& handler) const
{
    handler.send_element<types::message_id_t>(eventCodes::bombExploded);
    handler.send_element<types::bomb_id_t>(id);

    auto send_len = [&](types::vec_len_t t) {
        handler.send_element<types::vec_len_t>(t);
    };
    auto send_player_id = [&](const types::player_id_t& t) {
        handler.send_element<types::player_id_t>(t);
    };
    serialize_vector<types::message_id_t>(robots_destroyed, send_len, send_player_id);

    auto send_position = [&](const Position& t) {
        t.serialize(handler);
    };
    serialize_vector<Position>(blocks_destroyed, send_len, send_position);
}

PlayerMoved::PlayerMoved(TCPHandler& handler)
{
    id = handler.read_element<types::player_id_t>();
    position = Position(handler);
}

void PlayerMoved::serialize(TCPHandler& handler) const
{
    handler.send_element<types::message_id_t>(eventCodes::playerMoved);
    handler.send_element<types::player_id_t>(id);
    position.serialize(handler);
}

BlockPlaced::BlockPlaced(TCPHandler& handler)
{
    position = Position(handler);
}

void BlockPlaced::serialize(TCPHandler& handler) const
{
    handler.send_element<types::message_id_t>(eventCodes::blockPlaced);
    position.serialize(handler);
}

using Event = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

static Event read_event(TCPHandler& handler)
{
    types::message_id_t message_id = handler.read_element<types::message_id_t>();

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
    turn = handler.read_element<types::turn_t>();
    events = read_vector<Event>(handler, [&](){ return read_event(handler); });
}

void Turn::serialize(TCPHandler& handler) const
{
    handler.send_element<types::turn_t>(turn);
    auto send_len = [&](types::vec_len_t t) {
        handler.send_element<types::vec_len_t>(t);
    };
    auto send_event = [&](const Event& t) {
        std::visit([&](auto&& arg) {
            arg.serialize(handler);
        }, t);
    };
    serialize_vector<Event>(events, send_len, send_event);
}

GameEnded::GameEnded(TCPHandler& handler)
{
    scores = read_map<types::player_id_t, types::score_t>(handler,
             [&](){ return handler.read_element<types::player_id_t>(); }, 
             [&](){ return handler.read_element<types::score_t>(); });
}

void GameEnded::serialize(TCPHandler& handler) const
{
    auto send_len = [&](types::map_len_t t) {
        handler.send_element<types::map_len_t>(t);
    };
    auto send_player_id = [&](const types::player_id_t& t) {
        handler.send_element<types::player_id_t>(t);
    };
    auto send_score = [&](const types::score_t& t) {
        handler.send_element<types::score_t>(t);
    };
    serialize_map<types::player_id_t, types::score_t>(scores, send_len, send_player_id, send_score);
}

void Bomb::serialize(UDPHandler& handler) const
{
    position.serialize(handler);
    handler.append_to_outcoming_packet<types::bomb_timer_t>(timer);
}

void LobbyMessage::serialize(UDPHandler& handler) const
{
    // Serialize server name.
    auto send_str_len = [&](types::str_len_t t) {
        handler.append_to_outcoming_packet<types::str_len_t>(t);
    };
    auto send_char = [&](char t) {
        handler.append_to_outcoming_packet<char>(t);
    };
    serialize_string(server_name, send_str_len, send_char);

    // Serialize other fields.
    handler.append_to_outcoming_packet<types::players_count_t>(players_count);
    handler.append_to_outcoming_packet<types::size_xy_t>(size_x);
    handler.append_to_outcoming_packet<types::size_xy_t>(size_y);
    handler.append_to_outcoming_packet<types::game_length_t>(game_length);
    handler.append_to_outcoming_packet<types::explosion_radius_t>(explosion_radius);
    handler.append_to_outcoming_packet<types::bomb_timer_t>(bomb_timer);

    // Serialize map with players.
    auto send_map_len = [&](types::map_len_t t) {
        handler.append_to_outcoming_packet<types::map_len_t>(t);
    };
    auto send_key = [&](const types::player_id_t& t) {
        handler.append_to_outcoming_packet<types::player_id_t>(t);
    };
    auto send_val = [&](const Player& t) {
        t.serialize(handler);
    };
    serialize_map<types::player_id_t, Player>(players, send_map_len, send_key, send_val);
}

void GameMessage::serialize(UDPHandler& handler) const
{
    // Serialize server name.
    auto send_str_len = [&](types::str_len_t t) {
        handler.append_to_outcoming_packet<types::str_len_t>(t);
    };
    auto send_char = [&](char t) {
        handler.append_to_outcoming_packet<char>(t);
    };
    serialize_string(server_name, send_str_len, send_char);

    // Serialize other fields.
    handler.append_to_outcoming_packet<types::size_xy_t>(size_x);
    handler.append_to_outcoming_packet<types::size_xy_t>(size_y);
    handler.append_to_outcoming_packet<types::game_length_t>(game_length);
    handler.append_to_outcoming_packet<types::turn_t>(turn);

    // Serialize map with players.
    auto send_map_len = [&](types::map_len_t t) {
        handler.append_to_outcoming_packet<types::map_len_t>(t);
    };
    auto send_player_id = [&](const types::player_id_t& t) {
        handler.append_to_outcoming_packet<types::player_id_t>(t);
    };
    auto send_player = [&](Player t) {
        t.serialize(handler);
    };
    serialize_map<types::player_id_t, Player>(players, send_map_len, send_player_id, send_player);

    // Serialize map with player positions.
    auto send_position = [&](const Position& t) {
        t.serialize(handler);
    };
    serialize_map<types::player_id_t, Position>(player_positions, send_map_len, send_player_id, send_position);

    // Serialize vector with blocks.
    auto send_vec_len = [&](types::vec_len_t t) {
        handler.append_to_outcoming_packet<types::vec_len_t>(t);
    };
    serialize_vector<Position>(blocks, send_vec_len, send_position);

    // Serialize vector with bombs.
    auto send_bomb = [&](const Bomb& t) {
        t.serialize(handler);
    };
    serialize_vector<Bomb>(bombs, send_vec_len, send_bomb);

    // Serialize vector with explosions.
    serialize_vector<Position>(explosions, send_vec_len, send_position);

    // Serialize map with scores.
    auto send_score = [&](const types::score_t& t) {
        handler.append_to_outcoming_packet<types::score_t>(t);
    };
    serialize_map<types::player_id_t, types::score_t>(scores, send_map_len, send_player_id, send_score);
}
