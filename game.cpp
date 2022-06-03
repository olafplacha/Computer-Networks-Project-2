#include <chrono>
#include <thread>
#include "game.h"

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

void Game::decrease_bomb_timers()
{
    for(auto &[id, bomb] : bombs) {
        bomb.timer -= 1;
    }
}

GameClient::GameClient(const Hello& hello, const GameStarted& start)
{
    // Game settings.
    server_name = hello.server_name;
    size_x = hello.size_x;
    size_y = hello.size_y;
    game_length = hello.game_length;
    bomb_timer = hello.bomb_timer;
    explosion_radius = hello.explosion_radius;

    // Competitors.
    players = start.players;

    for(const auto& [id, player] : players) {
        // Initialize players' positions.
        Position p;
        player_positions.insert({id, p});
        // Initialize players' scores.
        scores.insert({id, 0});
    }
}

void GameClient::apply_event(const BombPlaced& event)
{
    // Add a new bomb.
    Bomb bomb;
    bomb.position = event.position;
    bomb.timer = bomb_timer;

    bombs.insert({event.id, bomb});
}

void GameClient::apply_event(const BombExploded& event)
{
    // Find the exploding bomb, mark exploded positions and erase the bomb.
    auto it = bombs.find(event.id);
    if (it != bombs.end()) {
        find_explosions(it->second);
        bombs.erase(it);
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

void GameClient::apply_event(const PlayerMoved& event)
{
    // Change player's position.
    auto it = player_positions.find(event.id);
    if (it != player_positions.end()) {
        it->second = event.position;
    }
}

void GameClient::apply_event(const BlockPlaced& event)
{
    // Add a new block.
    blocks.insert(event.position);
}

void Game::update_scores()
{
    for(const types::player_id_t& id : turn_robots_destroyed) {
        auto it = scores.find(id);
        if (it != scores.end()) {
            it->second += 1;
        }
    }
}

void GameClient::apply_turn(const Turn& turn_message)
{
    turn = turn_message.turn;
    decrease_bomb_timers();

    // Clear turn specific data structures.
    explosions.clear();
    turn_blocks_destroyed.clear();
    turn_robots_destroyed.clear();

    for (const Event& event : turn_message.events) {
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

GameMessage GameClient::get_game_state() const
{
    GameMessage message;
    message.server_name = server_name;
    message.size_x = size_x;
    message.size_y = size_y;
    message.game_length = game_length;
    message.turn = turn;
    message.players = players;
    message.player_positions = player_positions;

    std::vector<Position> blocks_vec;
    for (const Position& p : blocks) {
        blocks_vec.push_back(p);
    }
    message.blocks = blocks_vec;

    std::vector<Bomb> bombs_vec;
    for (auto const& [id, b] : bombs) {
        bombs_vec.push_back(b);
    }
    message.bombs = bombs_vec;

    std::vector<Position> explosions_vec;
    for (const Position& p : explosions) {
        explosions_vec.push_back(p);
    }
    message.explosions = explosions_vec;
    message.scores = scores;

    return message;
}

GameServer::GameServer(const options_server& op) : random(op.seed)
{
    server_name = op.server_name;
    size_x = op.size_x;
    size_y = op.size_y;
    game_length = op.game_length;
    bomb_timer = op.bomb_timer;
    explosion_radius = op.explosion_radius;
    
    turn = 0;
    turn_duration = op.turn_duration;
    initial_blocks = op.initial_blocks;

    // Initialize the map with scores.
    for (types::player_id_t i = 0; i < op.players_count; i++)
    {
        scores.insert({i, 0});
    }   
}

Turn GameServer::game_init()
{
    Turn turn_message;
    turn_message.turn = 0;

    for (types::player_id_t id = 0; id < players.size(); id++)
    {
        PlayerMoved event;

        Position position;
        position.x = random() % size_x;
        position.y = random() % size_y;

        event.id = id;
        event.position = position;
        turn_message.events.push_back(event);
    }

    for (types::initial_blocks_t i = 0; i < initial_blocks; i++)
    {
        Position position;
        position.x = random();
        position.y = random();

        auto it = blocks.insert(position);
        if (it.second) {
            // The block did not exist before.
            BlockPlaced event;
            event.position = position;

            turn_message.events.push_back(event);
        }
    }

    return turn_message;
}

Turn GameServer::apply_moves(MoveContainer& move_container)
{
    Turn turn_message;
    std::this_thread::sleep_for(std::chrono::milliseconds(turn_duration));

    turn_message.turn = ++turn;

    return turn_message;
}