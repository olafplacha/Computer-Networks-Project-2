#ifndef GAME_H
#define GAME_H

#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include "config.h"
#include "parser.h"
#include "messages.h"

class Game
{
public:
    /* Constructor used by the client. */
    Game(const Hello&, const GameStarted&);
    /* Constructor used by the server. */
    Game(const options_server&);
    /* Update game state based on the message received from the server. */
    void apply_turn(const Turn&);
    /* Get the state of the game. */
    GameMessage get_game_state() const;

private:
    void apply_event(const BombPlaced&);
    void apply_event(const BombExploded&);
    void apply_event(const PlayerMoved&);
    void apply_event(const BlockPlaced&);

    void decrease_bomb_timers();
    void update_scores();
    void find_explosions(const Bomb&);
    void explode_one_direction(const Position&, types::coord_t, types::coord_t);

    /* Game settings. */
    std::string server_name;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::bomb_timer_t bomber_timer;
    types::explosion_radius_t explosion_radius;

    /* Competitors. */
    std::map<types::player_id_t, Player> players;

    /* State of the game. */
    types::turn_t turn;
    std::map<types::player_id_t, Position> player_positions;
    std::unordered_set<Position, Position::HashFunction> blocks;
    std::unordered_map<types::bomb_id_t,Bomb> bombs;
    std::unordered_set<Position, Position::HashFunction> explosions;
    std::map<types::player_id_t, types::score_t> scores;
    
    /* Auxiliary data structures, cleared after each turn. */
    std::set<types::player_id_t> turn_robots_destroyed;
    std::unordered_set<Position, Position::HashFunction> turn_blocks_destroyed;
    std::set<Position> turn_explosions;
};

#endif // GAME_H