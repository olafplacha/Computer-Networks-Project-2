#ifndef GAME_H
#define GAME_H

#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include "config.h"
#include "parser.h"
#include "messages.h"
#include "move_container.h"

class Game
{
public:
    using score_map_t = std::map<types::player_id_t, types::score_t>;

protected:
    void decrease_bomb_timers();
    void find_explosions(const Bomb&);
    void explode_one_direction(const Position&, types::coord_t, types::coord_t);
    void update_scores();

    /* Game settings. */
    std::string server_name;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::bomb_timer_t bomb_timer;
    types::explosion_radius_t explosion_radius;

    /* Competitors. */
    std::map<types::player_id_t, Player> players;

    /* State of the game. */
    types::turn_t turn;
    std::map<types::player_id_t, Position> player_positions;
    std::unordered_set<Position, Position::HashFunction> blocks;
    std::unordered_map<types::bomb_id_t, Bomb> bombs;

    /* Score of each player. */
    score_map_t scores;
    
    /* Auxiliary data structures, cleared after each turn. */
    std::set<types::player_id_t> turn_robots_destroyed;
    std::unordered_set<Position, Position::HashFunction> turn_blocks_destroyed;
    std::unordered_set<Position, Position::HashFunction> explosions;
};

class GameClient : public Game 
{
public:
    /* Create the game based on the messages received from the server. */
    GameClient(const Hello&, const GameStarted&);

    /* Update game state based on the message received from the server. */
    void apply_turn(const Turn&);

    /* Get the state of the game. */
    GameMessage get_game_state() const;

private:
    void apply_event(const BombPlaced&);
    void apply_event(const BombExploded&);
    void apply_event(const PlayerMoved&);
    void apply_event(const BlockPlaced&);
};

class GameServer : public Game
{
public:
    /*  Create the game based on the provided options. */
    GameServer(const options_server&);

    Turn game_init();

    Turn apply_moves(MoveContainer&);
    score_map_t get_score_map() const;

private:
    bool is_position_legal(const Position&, types::coord_t, types::coord_t);
    void handle_exploding_bomb(types::bomb_id_t, Turn&);
    void handle_player_move(types::player_id_t, const ClientMessage&);

    void apply_player_move(types::player_id_t, Turn&, const Join&);
    void apply_player_move(types::player_id_t, Turn&, const PlaceBomb&);
    void apply_player_move(types::player_id_t, Turn&, const PlaceBlock&);
    void apply_player_move(types::player_id_t, Turn&, const Move&);

    void update_blocks();

    std::minstd_rand random;
    types::turn_duration_t turn_duration;
    types::initial_blocks_t initial_blocks;
    types::bomb_id_t bomb_counter;
};

#endif // GAME_H