#ifndef TURN_CONTAINER_H
#define TURN_CONTAINER_H

#include <vector>
#include <condition_variable>
#include <mutex>
#include "../config/config.h"
#include "../network/messages.h"
#include "../game_logic/game.h"

class TurnContainer {
public:
    using ptr = std::shared_ptr<TurnContainer>;

    TurnContainer() = default;

    void append_new_turn(const Turn &);

    Turn get_turn(types::turn_t);

    Game::score_map_t return_when_game_finished();

    /* Let other threads know that the game is finished and pass them the score map. */
    void mark_the_game_as_finished(const Game::score_map_t &);

    /* Delete copy constructor and copy assignment. */
    TurnContainer(TurnContainer const &) = delete;

    void operator=(TurnContainer const &) = delete;

private:
    std::mutex mutex;
    std::condition_variable condition_variable;
    std::vector<Turn> turns;
    Game::score_map_t score_map;

    bool finished = false;
};

#endif // TURN_CONTAINER_H