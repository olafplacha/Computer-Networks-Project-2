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
    TurnContainer() = default;

    void append_new_turn(const Turn&);

    Turn get_turn(types::turn_t);

private:
    std::mutex mutex;
    std::condition_variable condition_variable;
    std::vector<Turn> turns;
};

#endif // TURN_CONTAINER_H