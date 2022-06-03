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
    
    turn_duration = op.turn_duration;
    initial_blocks = op.initial_blocks;
    bomb_counter = 0;
    turn = 0;

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

    for (types::player_id_t id = 0; id < scores.size(); id++)
    {
        PlayerMoved event;

        Position position;
        position.x = random() % size_x;
        position.y = random() % size_y;
        player_positions.insert({id, position});

        event.id = id;
        event.position = position;
        turn_message.events.push_back(event);
    }

    for (types::initial_blocks_t i = 0; i < initial_blocks; i++)
    {
        BlockPlaced event;

        Position position;
        position.x = random() % size_x;
        position.y = random() % size_y;
        blocks.insert(position);

        event.position = position;
        turn_message.events.push_back(event);
    }

    return turn_message;
}

bool GameServer::is_position_legal(const Position& pos, types::coord_t dx, types::coord_t dy)
{
    // Check if the target position is within the board.
    if ((pos.x == 0 && dx < 0) || (pos.x + dx >= size_x)) {
        return false;
    }
    if ((pos.y == 0 && dy < 0) || (pos.y + dy >= size_y)) {
        return false;
    }

    // Check if the target position is not blocked.
    Position target;
    target.x = pos.x + dx;
    target.y = pos.y + dy;

    if (blocks.find(target) != blocks.end()) {
        return false;
    }

    // The target position is within the board and is not blocked.
    return true;
}

void GameServer::handle_exploding_bomb(types::bomb_id_t bomb_id, Turn& turn_message) 
{   
    // Get the exploding bomb.
    Bomb bomb = bombs.at(bomb_id);

    BombExploded event;
    event.id = bomb_id;

    // Find fields that explode.
    explosions.clear();
    find_explosions(bomb);

    // Mark exploded blocks.
    std::vector<Position> bomb_blocks_destroyed;

    for (const Position& pos : explosions) {
        if (blocks.find(pos) != blocks.end()) {
            bomb_blocks_destroyed.push_back(pos);
            turn_blocks_destroyed.insert(pos);
        }
    }

    // Mark exploded players.
    std::vector<types::player_id_t> bomb_robots_destroyed;

    for (const auto& [id, pos] : player_positions) {
        if (explosions.find(pos) != explosions.end()) {
            bomb_robots_destroyed.push_back(id);
            turn_robots_destroyed.insert(id);
        }
    }

    // Add the explosion event.
    event.blocks_destroyed = bomb_blocks_destroyed;
    event.robots_destroyed = bomb_robots_destroyed;
    turn_message.events.push_back(event);
}

void GameServer::apply_player_move(types::player_id_t, Turn&, const Join&)
{
    // Ignore.
}

void GameServer::apply_player_move(types::player_id_t id, Turn& turn_message, const PlaceBomb&)
{
    // Get the player's position.
    Position pos = player_positions.at(id);

    Bomb bomb;
    bomb.timer = bomb_timer;
    bomb.position = pos;

    // Add the newly placed bomb.
    bombs.insert({bomb_counter, bomb});

    // Create the event and add it to the turn message.
    BombPlaced event;
    event.id = bomb_counter++;
    event.position = pos;
    turn_message.events.push_back(event);
}

void GameServer::apply_player_move(types::player_id_t id, Turn& turn_message, const PlaceBlock&)
{
    // Get the player's position.
    Position pos = player_positions.at(id);

    // Check if there is already a block placed at this position.
    if (blocks.find(pos) != blocks.end()) {
        return;
    }

    blocks.insert(pos);

    // Create the event and add it to the turn message.
    BlockPlaced event;
    event.position = pos;
    turn_message.events.push_back(event);
}

void GameServer::apply_player_move(types::player_id_t id, Turn& turn_message, const Move& move)
{
    // Get the player's position.
    Position pos = player_positions.at(id);

    types::coord_t dx, dy;
    switch (move.direction)
    {
    case Direction::Up:
        dx = 0;
        dy = 1;
        break;

    case Direction::Right:
        dx = 1;
        dy = 0;
        break;

    case Direction::Down:
        dx = 0;
        dy = -1;
        break;
    
    default:
        dx = -1;
        dy = 0;
        break;
    }

    // Check if the target position is legal.
    bool legal = is_position_legal(pos, dx, dy);

    if (legal) {
        // Update player's position.
        pos.x += dx;
        pos.y += dy;
        player_positions.at(id) = pos;

        // And add it to the turn message.
        PlayerMoved event;
        event.id = id;
        event.position = pos;
        turn_message.events.push_back(event);
    }
}

void GameServer::update_blocks()
{
    for (const Position& pos : turn_blocks_destroyed) {
        blocks.erase(pos);
    }
}

Turn GameServer::apply_moves(MoveContainer& move_container)
{
    Turn turn_message;
    std::this_thread::sleep_for(std::chrono::milliseconds(turn_duration));

    // Get the last move from every player.
    MoveContainer::container_t moves = move_container.atomic_snapshot_and_clear();

    // Clear turn specific data structures.
    turn_blocks_destroyed.clear();
    turn_robots_destroyed.clear();

    // Check what bombs explode.
    decrease_bomb_timers();

    for (auto it = bombs.cbegin(), next_it = it; it != bombs.cend(); it = next_it) {
        ++next_it;
        if (it->second.timer == 0) {
            // The bomb explodes.
            handle_exploding_bomb(it->first, turn_message);

            // Erase the bomb after explosion.
            bombs.erase(it);
        }
    }

    for (types::player_id_t i = 0; i < scores.size(); i++)
    {
        // Check if the player way destroyed.
        if (turn_robots_destroyed.find(i) != turn_robots_destroyed.end()) {
            // Recreate the player in random position.
            Position pos;
            pos.x = random() % size_x;
            pos.y = random() % size_y;

            player_positions.at(i) = pos;

            PlayerMoved event;
            event.id = i;
            event.position = pos;
            turn_message.events.push_back(event);
        }
        else {
            // Handle the player's move.
            auto [updated, move] = moves.at(i);

            // Check if the move was updated from the last snapshot.
            if (updated) {
                // Apply player's move.
                std::visit([&](auto&& arg) {apply_player_move(i, turn_message, arg);}, move);
            }
        }
    }
    
    update_blocks();
    update_scores();
    turn_message.turn = ++turn;

    return turn_message;
}