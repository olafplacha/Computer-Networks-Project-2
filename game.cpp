#include "game.h"

Game::Game(const Hello& hello, const GameStarted& start)
{
    // Game settings.
    server_name = hello.server_name;
    size_x = hello.size_x;
    size_y = hello.size_y;
    game_length = hello.game_length;
    bomber_timer = hello.bomber_timer;
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

void Game::apply_event(const BombPlaced& event)
{
    // Add a new bomb.
    Bomb bomb;
    bomb.position = event.position;
    bomb.timer = bomber_timer;

    bombs.insert({event.id, bomb});
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

void Game::apply_event(const BombExploded& event)
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

void Game::apply_event(const PlayerMoved& event)
{
    // Change player's position.
    auto it = player_positions.find(event.id);
    if (it != player_positions.end()) {
        it->second = event.position;
    }
}

void Game::apply_event(const BlockPlaced& event)
{
    // Add a new block.
    blocks.insert(event.position);
}

void Game::decrease_bomb_timers()
{
    for(auto &[id, bomb] : bombs) {
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
}

void Game::apply_turn(const Turn& turn_message)
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

GameMessage Game::get_game_state() const
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