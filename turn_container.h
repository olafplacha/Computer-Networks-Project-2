#ifndef TURN_CONTAINER_H
#define TURN_CONTAINER_H

#include <vector>
#include <condition_variable>
#include <mutex>
#include "config.h"
#include "messages.h"

class TurnContainer
{
public:
    using ptr = std::shared_ptr<TurnContainer>;

    TurnContainer() = default;

    void append_new_turn(const Turn&);

    Turn get_turn(types::turn_t);

    void return_when_game_finished();

    void mark_the_game_as_finished();

    /* Delete copy constructor and copy assignment. */
    TurnContainer(TurnContainer const &) = delete;
    void operator=(TurnContainer const &) = delete;

private:
    std::mutex mutex;
    std::condition_variable condition_variable;
    std::vector<Turn> turns;
    bool finished = false;
};

#endif // TURN_CONTAINER_H