#ifndef MOVE_CONTAINER_H
#define MOVE_CONTAINER_H

#include <vector>
#include <utility>
#include <condition_variable>
#include <mutex>
#include "config.h"
#include "messages.h"

class MoveContainer
{
public:
    /* Holds information about the last move and whether it was updated from the last snapshot. */
    using container_t = std::vector<std::pair<bool, ClientMessage>>;

    MoveContainer(types::player_id_t);

    container_t atomic_snapshot_and_clear();
    
    void update_slot(types::player_id_t, const ClientMessage&);

private:
    std::mutex mutex;
    container_t slots;
};

#endif // MOVE_CONTAINER_H