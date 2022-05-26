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
    // Truncate string if too long.
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

template<typename K, typename V>
void serialize_map(std::map<K, V>& m, std::function<void (uint32_t)> send_int,
                   std::function<void (K)> send_key, std::function<void (V)> send_val)
{
    // Maps of bigger size are not supported.
    uint32_t n = (uint32_t) m.size();

    send_int(n);
    for (auto const& [key, value] : m) {
        send_key(key);
        send_val(value);
    }
}

template<typename T>
void serialize_vector(std::vector<T>& v, std::function<void (uint32_t)> send_int,
                      std::function<void (T)> send_element)
{
    // Vectors of bigger size are not supported.
    uint32_t n = (uint32_t) v.size();

    send_int(n);
    for (auto const& element : v) {
        send_element(element);
    }
}

Join::Join(std::string& name_)
{
    name = name_;
}

void Join::serialize(TCPHandler& handler)
{
    serialize_string(name, [&](uint8_t t) {
        handler.send_element<uint8_t>(t);
    },
    [&](char t) {
        handler.send_element<char>(t);
    });
}

Move::Move(Direction direction_) : direction(direction_) {}

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

void Player::serialize(UDPHandler& handler)
{
    auto send_int = [&](uint8_t t) {
        handler.append_to_outcoming_packet<uint8_t>(t);
    };
    auto send_char = [&](char t) {
        handler.append_to_outcoming_packet<char>(t);
    };
    serialize_string(name, send_int, send_char);
    serialize_string(address, send_int, send_char);
}

AcceptedPlayer::AcceptedPlayer(TCPHandler& handler)
{
    id = read_element<types::player_id_t>(handler);
    player = Player(handler);
}

GameStarted::GameStarted(TCPHandler& handler)
{
    players = read_map<types::player_id_t, Player>(handler, read_element<types::player_id_t>,
    [&](TCPHandler& t) {
        return Player(t);
    });
}

Position::Position(TCPHandler& handler)
{
    x = read_element<types::size_xy_t>(handler);
    y = read_element<types::size_xy_t>(handler);
}

bool Position::operator==(const Position& rhs) const
{
    return x == rhs.x && y == rhs.y;
}

void Position::serialize(UDPHandler& handler)
{
    handler.append_to_outcoming_packet<types::size_xy_t>(x);
    handler.append_to_outcoming_packet<types::size_xy_t>(y);
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
    blocks_destroyed = read_vector<Position>(handler, [&](TCPHandler& t) {
        return Position(t);
    });
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

void Bomb::serialize(UDPHandler& handler)
{
    position.serialize(handler);
    handler.append_to_outcoming_packet<types::bomb_timer_t>(timer);
}

Lobby::Lobby(Hello& hello)
{
    server_name = hello.server_name;
    players_count = hello.players_count;
    size_x = hello.size_x;
    size_y = hello.size_y;
    game_length = hello.game_length;
    explosion_radius = hello.explosion_radius;
    bomb_timer = hello.bomber_timer;
}

void Lobby::accept(AcceptedPlayer& player)
{
    players.insert({player.id, player.player});
}

void Lobby::serialize(UDPHandler& handler)
{
    // Serialize server name.
    auto send_int8 = [&](uint8_t t) {
        handler.append_to_outcoming_packet<uint8_t>(t);
    };
    auto send_char = [&](char t) {
        handler.append_to_outcoming_packet<char>(t);
    };
    serialize_string(server_name, send_int8, send_char);

    // Serialize other fields.
    handler.append_to_outcoming_packet<types::players_count_t>(players_count);
    handler.append_to_outcoming_packet<types::size_xy_t>(size_x);
    handler.append_to_outcoming_packet<types::size_xy_t>(size_y);
    handler.append_to_outcoming_packet<types::game_length_t>(game_length);
    handler.append_to_outcoming_packet<types::explosion_radius_t>(explosion_radius);
    handler.append_to_outcoming_packet<types::bomb_timer_t>(bomb_timer);

    // Serialize map with players.
    auto send_int32 = [&](uint32_t t) {
        handler.append_to_outcoming_packet<uint32_t>(t);
    };
    auto send_key = [&](types::player_id_t t) {
        handler.append_to_outcoming_packet<types::player_id_t>(t);
    };
    auto send_val = [&](Player t) {
        t.serialize(handler);
    };
    serialize_map<types::player_id_t, Player>(players, send_int32, send_key, send_val);
}

Game::Game(Hello& hello, GameStarted& start)
{
    server_name = hello.server_name;
    size_x = hello.size_x;
    size_y = hello.size_y;
    game_length = hello.game_length;
    turn = 0;
    players = start.players;

    for(const auto& [id, player] : players) {
        // Initialize players' positions.
        Position p;
        player_positions.insert({id, p});
        // Initialize players' scores.
        scores.insert({id, 0});
    }
    bomber_timer = hello.bomber_timer;
    explosion_radius = hello.explosion_radius;
}

void Game::apply_event(BombPlaced& event)
{
    // Add a new bomb.
    Bomb bomb;
    bomb.id = event.id;
    bomb.position = event.position;
    bomb.timer = bomber_timer;

    bombs.push_back(bomb);
}

void Game::explode_one_direction(const Position& pos, types::coord_t dx, types::coord_t dy)
{
    types::coord_t curr_x = pos.x;
    types::coord_t curr_y = pos.y;
    types::explosion_radius_t curr_radius = 0;

    while (curr_x >= 0 && curr_x < size_x && curr_y >= 0 && curr_y < size_y && curr_radius <= explosion_radius) {
        // Casting is safe because of the above condition.
        Position p;
        p.x = (types::size_xy_t) curr_x;
        p.y = (types::size_xy_t) curr_y;

        // Add current position to explosions.
        explosions.insert(p);

        if (blocks.find(p) != blocks.end()) {
            // Current position is blocked.
            return;
        }
        curr_x += dx;
        curr_y += dy;
        curr_radius++;
    }
}

void Game::find_explosions(const Bomb& bomb)
{
    Position pos = bomb.position;
    explode_one_direction(pos, 0, 1);
    explode_one_direction(pos, 1, 0);
    explode_one_direction(pos, 0, -1);
    explode_one_direction(pos, -1, 0);
}

void Game::apply_event(BombExploded& event)
{
    // Find the exploding bomb.
    for (size_t i = 0; i < bombs.size(); i++)
    {
        if (event.id == bombs.at(i).id) {
            // Find the exploded positions.
            find_explosions(bombs.at(i));
            // Remove the bomb.
            bombs.erase(bombs.begin() + i);
            break;
        }
    }

    // Mark destroyed robots.
    for (const types::player_id_t& id : event.robots_destroyed) {
        turn_robots_destroyed.insert(id);
    }

    // Add removed blocks.
    for (const Position& pos : event.blocks_destroyed) {
        turn_blocks_destroyed.insert(pos);
    }
}

void Game::apply_event(PlayerMoved& event)
{
    // Change player's position.
    auto it = player_positions.find(event.id);
    if (it != player_positions.end()) {
        it->second = event.position;
    }
}

void Game::apply_event(BlockPlaced& event)
{
    // Add a new block.
    blocks.insert(event.position);
}

void Game::decrease_bomb_timers()
{
    for(Bomb& bomb : bombs) {
        bomb.timer -= 1;
    }
}

void Game::update_scores()
{
    for(const types::player_id_t& id : turn_robots_destroyed) {
        auto it = scores.find(id);
        if (it != scores.end()) {
            it->second += 1;
        }
    }

    // Clear the set for the next turn.
    turn_robots_destroyed.clear();
}

void Game::apply_turn(Turn& turn_message)
{
    turn = turn_message.turn;
    decrease_bomb_timers();

    // Clear explosion set from the previous turn.
    explosions.clear();
    // Clear destroyed blocks from the previous turn.
    turn_blocks_destroyed.clear();

    for (Event& event : turn_message.events) {
        // Apply each event to the state of the game.
        std::visit([&](auto&& arg) {
            apply_event(arg);
        }, event);
    }

    // After all bombs exploded, remove destroyed blocks.
    for (const Position& pos : turn_blocks_destroyed) {
        blocks.erase(pos);
    }

    update_scores();
}

void Game::serialize(UDPHandler& handler)
{
    // Serialize server name.
    auto send_int8 = [&](uint8_t t) {
        handler.append_to_outcoming_packet<uint8_t>(t);
    };
    auto send_char = [&](char t) {
        handler.append_to_outcoming_packet<char>(t);
    };
    serialize_string(server_name, send_int8, send_char);

    // Serialize other fields.
    handler.append_to_outcoming_packet<types::size_xy_t>(size_x);
    handler.append_to_outcoming_packet<types::size_xy_t>(size_y);
    handler.append_to_outcoming_packet<types::game_length_t>(game_length);
    handler.append_to_outcoming_packet<types::turn_t>(turn);

    // Serialize map with players.
    auto send_int32 = [&](uint32_t t) {
        handler.append_to_outcoming_packet<uint32_t>(t);
    };
    auto send_key = [&](types::player_id_t t) {
        handler.append_to_outcoming_packet<types::player_id_t>(t);
    };
    auto send_player = [&](Player t) {
        t.serialize(handler);
    };
    serialize_map<types::player_id_t, Player>(players, send_int32, send_key, send_player);

    // Serialize map with players positions.
    auto send_position = [&](Position t) {
        t.serialize(handler);
    };
    serialize_map<types::player_id_t, Position>(player_positions, send_int32, send_key, send_position);

    // Serialize vector with blocks positions.
    std::vector<Position> blocks_vec;
    for (const Position& p : blocks) {
        blocks_vec.push_back(p);
    }
    serialize_vector<Position>(blocks_vec, send_int32, send_position);

    // Serialize vector with bombs.
    auto send_bomb = [&](Bomb t) {
        t.serialize(handler);
    };
    serialize_vector<Bomb>(bombs, send_int32, send_bomb);

    // Serialize vector with explosions.
    std::vector<Position> explosions_vec;
    for (const Position& p : explosions) {
        explosions_vec.push_back(p);
    }
    serialize_vector<Position>(explosions_vec, send_int32, send_position);

    // Serialize map with scores.
    auto send_score = [&](types::score_t t) {
        handler.append_to_outcoming_packet<types::score_t>(t);
    };
    serialize_map<types::player_id_t, types::score_t>(scores, send_int32, send_key, send_score);
}

ClientMessageManager::ClientMessageManager(TCPHandler& tcp_handler_, UDPHandler& udp_handler_) :
    tcp_handler(tcp_handler_), udp_handler(udp_handler_) {}

ServerMessage ClientMessageManager::read_server_message()
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

InputMessage ClientMessageManager::read_gui_message()
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

void ClientMessageManager::send_server_message(Join& message)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::join);
    message.serialize(tcp_handler);
}

void ClientMessageManager::send_server_message(PlaceBomb&)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::placeBomb);
}

void ClientMessageManager::send_server_message(PlaceBlock&)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::placeBlock);
}

void ClientMessageManager::send_server_message(Move& message)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::move);
    message.serialize(tcp_handler);
}

// Ignore.
void ClientMessageManager::send_server_message(InvalidMessage&) {};

// Over UDP.
void ClientMessageManager::send_gui_message(Lobby& message)
{
    udp_handler.append_to_outcoming_packet<types::message_id_t>(guiClientCodes::lobby);
    message.serialize(udp_handler);

    // Send the packet.
    udp_handler.flush_outcoming_packet();
}

void ClientMessageManager::send_gui_message(Game& message)
{
    udp_handler.append_to_outcoming_packet<types::message_id_t>(guiClientCodes::game);
    message.serialize(udp_handler);

    // Send the packet.
    udp_handler.flush_outcoming_packet();
}
